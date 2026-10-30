// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/TrayController.h"

#include "controllers/PhoneController.h"
#include "settings/AccountManager.h"

#include <QAction>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QWindow>

namespace {

constexpr auto kHintShownKey = "tray/hintShown";
constexpr int kMessageTimeoutMs = 6000;

QColor statusColor(const QString &status)
{
    if (status == QLatin1String("registered"))
        return QColor::fromRgb(0x22C55E);
    if (status == QLatin1String("registering"))
        return QColor::fromRgb(0xF59E0B);
    if (status == QLatin1String("failed"))
        return QColor::fromRgb(0xEF4444);
    return QColor::fromRgb(0x8D97A6);
}

} // namespace

TrayController::TrayController(PhoneController *phone, AccountManager *accounts, QObject *parent)
    : QObject(parent)
    , m_phone(phone)
    , m_accounts(accounts)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    // Closing the window only hides it; the tray keeps the app and its registration alive.
    QGuiApplication::setQuitOnLastWindowClosed(false);

    m_menu = std::make_unique<QMenu>();
    connect(m_menu->addAction(tr("Show Silent Softphone Free")), &QAction::triggered, this, &TrayController::showWindow);
    m_menu->addSeparator();
    m_accountsMenu = m_menu->addMenu(tr("Account"));
    connect(m_menu->addAction(tr("Settings…")), &QAction::triggered, this, [this] {
        showWindow();
        emit settingsRequested();
    });
    m_menu->addSeparator();
    connect(m_menu->addAction(tr("Quit")), &QAction::triggered, this, &TrayController::quit);

    m_tray = new QSystemTrayIcon(this);
    m_tray->setContextMenu(m_menu.get());
    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
#ifdef Q_OS_MACOS
        Q_UNUSED(reason) // a click on the menu bar icon opens the menu
#else
        if (reason == QSystemTrayIcon::Trigger)
            toggleWindow();
#endif
    });
    connect(m_tray, &QSystemTrayIcon::messageClicked, this, &TrayController::showWindow);

    connect(m_phone, &PhoneController::registrationChanged, this, &TrayController::updateIcon);
    connect(m_phone, &PhoneController::missedCall, this, &TrayController::onMissedCall);
    // Queued: a menu action may trigger these while it is still being delivered,
    // and rebuilding deletes the actions.
    connect(m_phone, &PhoneController::currentCallChanged, this, &TrayController::rebuildAccountsMenu, Qt::QueuedConnection);
    connect(m_accounts, &AccountManager::countChanged, this, &TrayController::rebuildAccountsMenu, Qt::QueuedConnection);
    connect(m_accounts, &AccountManager::currentChanged, this, &TrayController::rebuildAccountsMenu, Qt::QueuedConnection);
    connect(m_accounts, &AccountManager::dataChanged, this, &TrayController::rebuildAccountsMenu, Qt::QueuedConnection);

    rebuildAccountsMenu();
    updateIcon();
    m_tray->show();
}

TrayController::~TrayController() = default;

void TrayController::attach(QWindow *window)
{
    m_window = window;
    if (!window)
        return;
    connect(window, &QWindow::activeChanged, this, [this] {
        if (m_window && m_window->isActive())
            clearMissedCalls();
    });
#ifdef Q_OS_MACOS
    // Clicking the Dock icon activates the app with no window focused: bring the
    // hidden window back, as Mac apps do. (Clicking the call popup focuses the popup.)
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive && m_window && !m_window->isVisible()
            && !QGuiApplication::focusWindow())
            showWindow();
    });
#endif
}

void TrayController::showWindow()
{
    if (!m_window)
        return;
    if (m_window->visibility() == QWindow::Minimized)
        m_window->showNormal();
    else
        m_window->show();
    m_window->raise();
    m_window->requestActivate();
    clearMissedCalls();
}

void TrayController::hideWindow()
{
    if (m_window)
        m_window->hide();
}

void TrayController::notifyHiddenToTray()
{
    if (!m_tray || m_quitting)
        return;
    QSettings settings;
    if (settings.value(kHintShownKey, false).toBool())
        return;
    settings.setValue(kHintShownKey, true);
    m_tray->showMessage(tr("Silent Softphone Free is still running"),
                        tr("Calls will still ring. Use the tray icon to open the app or quit."),
                        QSystemTrayIcon::Information, kMessageTimeoutMs);
}

void TrayController::quit()
{
    m_quitting = true;
    QCoreApplication::quit();
}

QIcon TrayController::statusIcon(const QString &status, int missedCalls)
{
    QIcon icon;
    for (const int size : { 16, 20, 24, 32, 40, 48, 64 })
        icon.addPixmap(QPixmap::fromImage(renderStatusIcon(status, missedCalls, size)));
    return icon;
}

QImage TrayController::renderStatusIcon(const QString &status, int missedCalls, int size)
{
    static const QImage appIcon(QStringLiteral(":/assets/icon/app.png"));

    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRectF(0, 0, size, size), appIcon);

    // Status dot, bottom-right, with a white ring so it reads on any taskbar color.
    const qreal ring = qMax(1.0, size / 16.0);
    const qreal dot = size * 0.42;
    painter.setPen(QPen(Qt::white, ring));
    painter.setBrush(statusColor(status));
    painter.drawEllipse(QRectF(size - dot, size - dot, dot, dot).adjusted(ring / 2, ring / 2, -ring / 2, -ring / 2));

    // Missed calls: red badge, top-right; the count only where it is legible.
    if (missedCalls > 0) {
        const qreal badge = size * 0.5;
        const QRectF badgeRect = QRectF(size - badge, 0, badge, badge).adjusted(ring / 2, ring / 2, -ring / 2, -ring / 2);
        painter.setBrush(QColor::fromRgb(0xEF4444));
        painter.drawEllipse(badgeRect);
        if (size >= 24) {
            QFont font = painter.font();
            font.setBold(true);
            font.setPixelSize(int(badge * 0.62));
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(badgeRect, Qt::AlignCenter,
                             missedCalls > 9 ? QStringLiteral("9+") : QString::number(missedCalls));
        }
    }
    return image;
}

void TrayController::updateIcon()
{
    if (!m_tray)
        return;
    m_tray->setIcon(statusIcon(m_phone->registrationStatus(), m_missedCalls));
    QString toolTip = QStringLiteral("Silent Softphone Free\n") + m_phone->registrationText();
    if (m_missedCalls > 0)
        toolTip += u'\n' + tr("Missed calls: %1").arg(m_missedCalls);
    m_tray->setToolTip(toolTip);
}

void TrayController::rebuildAccountsMenu()
{
    if (!m_accountsMenu)
        return;
    m_accountsMenu->clear();
    const bool canSwitch = !m_phone->inCall();
    for (int row = 0; row < m_accounts->count(); ++row) {
        const QString label = m_accounts->data(m_accounts->index(row), AccountManager::LabelRole).toString();
        QAction *action = m_accountsMenu->addAction(label);
        action->setCheckable(true);
        action->setChecked(row == m_accounts->currentIndex());
        action->setEnabled(canSwitch);
        connect(action, &QAction::triggered, this, [this, row] { m_accounts->activate(row); });
    }
    m_accountsMenu->menuAction()->setVisible(m_accounts->count() > 1);
}

void TrayController::onMissedCall(const QString &caller)
{
    if (m_window && m_window->isActive())
        return; // the user is looking at the app
    ++m_missedCalls;
    updateIcon();
    if (m_tray)
        m_tray->showMessage(tr("Missed call"), caller, QSystemTrayIcon::Information, kMessageTimeoutMs);
}

void TrayController::toggleWindow()
{
    if (m_window && m_window->isVisible() && m_window->visibility() != QWindow::Minimized)
        hideWindow();
    else
        showWindow();
}

void TrayController::clearMissedCalls()
{
    if (m_missedCalls == 0)
        return;
    m_missedCalls = 0;
    updateIcon();
}
