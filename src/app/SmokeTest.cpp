// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/SmokeTest.h"

#include "app/Application.h"
#include "app/Diagnostics.h"
#include "controllers/PhoneController.h"
#include "core/CallSession.h"
#include "ui/ThemeManager.h"
#include "ui/TrayController.h"

#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include <cstdio>

namespace {

constexpr int kTickMs = 100;
constexpr int kStepTimeoutMs = 15000;
constexpr int kSettleMs = 300; // let bindings, layouts and animations settle before a screenshot

int g_qmlWarnings = 0;

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message)
{
    std::fprintf(stderr, "%s\n", qPrintable(message));
    std::fflush(stderr);
    if (type != QtDebugMsg && type != QtInfoMsg && message.contains(QLatin1String(".qml")))
        ++g_qmlWarnings;
}

} // namespace

SmokeTest::SmokeTest(Application &application, QQmlApplicationEngine &engine, const QString &screenshotDir)
    : m_application(application)
    , m_engine(engine)
    , m_screenshotDir(screenshotDir)
{
    if (!m_screenshotDir.isEmpty())
        QDir().mkpath(m_screenshotDir);
    m_ticker.setInterval(kTickMs);
    connect(&m_ticker, &QTimer::timeout, this, &SmokeTest::tick);
}

void SmokeTest::installMessageHandler()
{
    qInstallMessageHandler(messageHandler);
}

void SmokeTest::start()
{
    PhoneController *phone = m_application.phone();
    ThemeManager *theme = m_application.theme();
    const ThemeManager::Mode originalTheme = theme->mode();

    theme->setMode(ThemeManager::Light);
    saveTrayIcons();

    // Outgoing call: connect, DTMF, mute, hold, hang up.
    waitFor(QStringLiteral("registered"), [phone] { return phone->registrationStatus() == QLatin1String("registered"); },
            [phone] { phone->dial(QStringLiteral("1002")); });
    waitFor(QStringLiteral("outgoing call connected"), [this] { return callIn(CallSession::Active); }, {});
    after(kSettleMs, QStringLiteral("call screenshot"), [this, phone] {
        screenshot(QStringLiteral("1-call-light"));
        phone->sendDtmf(QStringLiteral("5"));
        phone->toggleMute();
        phone->toggleHold();
    });
    waitFor(QStringLiteral("call on hold"), [this] { return callIn(CallSession::Held); },
            [phone] { phone->hangup(); });

    // Incoming call: the popup must appear; answer, then hang up.
    waitFor(QStringLiteral("idle after first call"), [this] { return idle(); },
            [this] { m_application.diagnostics()->simulateIncomingCall(); });
    waitFor(QStringLiteral("incoming popup shown"), [this] { return callIn(CallSession::Alerting) && popupVisible(); }, {});
    after(kSettleMs, QStringLiteral("incoming screenshot"), [this, phone] {
        screenshot(QStringLiteral("2-incoming-light"));
        phone->answer();
    });
    waitFor(QStringLiteral("incoming call connected"), [this] { return callIn(CallSession::Active) && !popupVisible(); },
            [phone] {
                phone->sendDtmf(QStringLiteral("1"));
                phone->hangup();
            });

    // Settings with a ComboBox popup open, in both themes.
    waitFor(QStringLiteral("idle after incoming call"), [this] { return idle(); }, [this] {
        setRootProperty("height", 1900);
        setRootProperty("currentTab", 3);
        setComboPopupOpen("transportBox", true);
    });
    after(kSettleMs, QStringLiteral("settings light"), [this, theme] {
        screenshot(QStringLiteral("3-settings-light"));
        theme->setMode(ThemeManager::Dark);
    });
    after(kSettleMs, QStringLiteral("settings dark"), [this, phone] {
        screenshot(QStringLiteral("3-settings-dark"));
        setComboPopupOpen("transportBox", false);
        setRootProperty("currentTab", 0);
        setRootProperty("height", 640); // the window's minimum height
        phone->dial(QStringLiteral("700"));
    });

    // Call layout at the minimum window height, then the wide layout.
    waitFor(QStringLiteral("second call connected"), [this] { return callIn(CallSession::Active); }, {});
    after(kSettleMs, QStringLiteral("call at minimum height"), [this] {
        screenshot(QStringLiteral("4-call-dark-min-height"));
        setRootProperty("width", 900);
    });
    after(kSettleMs, QStringLiteral("wide layout"), [this, phone] {
        screenshot(QStringLiteral("5-call-wide-dark"));
        phone->hangup();
    });
    waitFor(QStringLiteral("idle at the end"), [this] { return idle(); }, [theme, originalTheme] {
        theme->setMode(originalTheme);
    });

    m_stepClock.start();
    m_ticker.start();
}

void SmokeTest::waitFor(const QString &name, std::function<bool()> ready, std::function<void()> action)
{
    m_steps.append({ name, std::move(ready), std::move(action), 0 });
}

void SmokeTest::after(int delayMs, const QString &name, std::function<void()> action)
{
    m_steps.append({ name, [] { return true; }, std::move(action), delayMs });
}

void SmokeTest::tick()
{
    if (m_current >= m_steps.size()) {
        finish(g_qmlWarnings == 0, QStringLiteral("QML warnings: %1").arg(g_qmlWarnings));
        return;
    }

    const Step &step = m_steps.at(m_current);
    const qint64 elapsed = m_stepClock.elapsed();
    if (elapsed >= step.minimumWaitMs && step.ready()) {
        std::fprintf(stderr, "smoke step ok: %s (%lld ms)\n", qPrintable(step.name), elapsed);
        if (step.action)
            step.action();
        ++m_current;
        m_stepClock.restart();
    } else if (elapsed > kStepTimeoutMs) {
        finish(false, QStringLiteral("step '%1' timed out").arg(step.name));
    }
}

void SmokeTest::finish(bool success, const QString &message)
{
    m_ticker.stop();
    std::fprintf(stderr, "smoke test %s: %s\n", success ? "finished" : "FAILED", qPrintable(message));
    QCoreApplication::exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
}

bool SmokeTest::callIn(int state) const
{
    const CallSession *call = m_application.phone()->currentCall();
    return call && call->state() == state;
}

bool SmokeTest::idle() const
{
    return !m_application.phone()->inCall();
}

bool SmokeTest::popupVisible() const
{
    const QQuickWindow *popup = popupWindow();
    return popup && popup->isVisible();
}

QObject *SmokeTest::root() const
{
    const auto roots = m_engine.rootObjects();
    return roots.isEmpty() ? nullptr : roots.first();
}

QQuickWindow *SmokeTest::popupWindow() const
{
    for (QWindow *window : QGuiApplication::topLevelWindows()) {
        if (window->objectName() == QLatin1String("incomingCallWindow"))
            return qobject_cast<QQuickWindow *>(window);
    }
    return nullptr;
}

void SmokeTest::screenshot(const QString &name) const
{
    if (m_screenshotDir.isEmpty())
        return;
    const auto save = [this](QQuickWindow *window, const QString &file) {
        const QString path = m_screenshotDir + u'/' + file + QStringLiteral(".png");
        if (!window->grabWindow().save(path))
            std::fprintf(stderr, "screenshot failed: %s\n", qPrintable(path));
    };
    if (auto *window = qobject_cast<QQuickWindow *>(root()))
        save(window, name);
    if (QQuickWindow *popup = popupWindow(); popup && popup->isVisible())
        save(popup, name + QStringLiteral("-popup"));
}

// The tray icon in every status, at taskbar (32 px) and high-DPI (64 px) sizes, on
// light and dark backgrounds; the real tray can't be captured offscreen.
void SmokeTest::saveTrayIcons() const
{
    if (m_screenshotDir.isEmpty())
        return;
    const QStringList states = { QStringLiteral("registered"), QStringLiteral("registering"),
                                 QStringLiteral("failed"), QStringLiteral("offline"),
                                 QStringLiteral("registered") };
    const int columnWidth = 120;
    QImage sheet(columnWidth * int(states.size()) + 16, 2 * 80, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&sheet);
    painter.fillRect(0, 0, sheet.width(), 80, QColor::fromRgb(0xF4F6F9));
    painter.fillRect(0, 80, sheet.width(), 80, QColor::fromRgb(0x0F1318));
    for (int row = 0; row < 2; ++row) {
        for (int i = 0; i < states.size(); ++i) {
            const int missed = i == states.size() - 1 ? 3 : 0;
            const int x = 16 + i * columnWidth;
            const int y = row * 80 + 8;
            painter.drawImage(x, y, TrayController::renderStatusIcon(states.at(i), missed, 64));
            painter.drawImage(x + 72, y + 16, TrayController::renderStatusIcon(states.at(i), missed, 32));
        }
    }
    painter.end();
    sheet.save(m_screenshotDir + QStringLiteral("/6-tray-icons.png"));
}

void SmokeTest::setComboPopupOpen(const char *objectName, bool open) const
{
    QObject *combo = root() ? root()->findChild<QObject *>(QLatin1String(objectName)) : nullptr;
    QObject *popup = combo ? combo->property("popup").value<QObject *>() : nullptr;
    if (popup)
        QMetaObject::invokeMethod(popup, open ? "open" : "close");
}

void SmokeTest::setRootProperty(const char *name, const QVariant &value) const
{
    if (QObject *object = root())
        object->setProperty(name, value);
}
