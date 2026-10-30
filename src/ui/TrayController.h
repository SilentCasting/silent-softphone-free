// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIcon>
#include <QImage>
#include <QObject>
#include <QPointer>

#include <memory>

class AccountManager;
class PhoneController;
class QMenu;
class QSystemTrayIcon;
class QWindow;

// System tray / menu bar icon: registration status, missed-call badge, and a menu to
// show the window, switch accounts, open settings or quit. When a tray is available
// the app keeps running after its window is closed, so it can still take calls.
class TrayController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable CONSTANT)

public:
    TrayController(PhoneController *phone, AccountManager *accounts, QObject *parent = nullptr);
    ~TrayController() override;

    bool isAvailable() const { return m_tray != nullptr; }

    Q_INVOKABLE void attach(QWindow *window);
    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    // Called when the window was closed into the tray; explains it once.
    Q_INVOKABLE void notifyHiddenToTray();
    Q_INVOKABLE void quit();

    // App icon with a status dot ("registered" | "registering" | "failed" | "offline")
    // and an optional missed-call badge.
    static QIcon statusIcon(const QString &status, int missedCalls);
    static QImage renderStatusIcon(const QString &status, int missedCalls, int size);

signals:
    void settingsRequested();

private:
    void updateIcon();
    void rebuildAccountsMenu();
    void onMissedCall(const QString &caller);
    void toggleWindow();
    void clearMissedCalls();

    PhoneController *m_phone;
    AccountManager *m_accounts;
    QPointer<QWindow> m_window;
    std::unique_ptr<QMenu> m_menu;
    QMenu *m_accountsMenu = nullptr;
    QSystemTrayIcon *m_tray = nullptr;
    int m_missedCalls = 0;
    bool m_quitting = false;
};
