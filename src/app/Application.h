// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class AccountManager;
class AutoStart;
class CallHistoryModel;
class CallSettings;
class CodecModel;
class ContactFilterModel;
class ContactModel;
class Diagnostics;
class ISipService;
class PhoneController;
class QQmlApplicationEngine;
class ThemeManager;
class TrayController;
class WindowSettings;

// Composition root: builds the object graph and exposes it to QML as
// singletons of the "SilentSoftphone.Core" module.
class Application : public QObject
{
    Q_OBJECT

public:
    enum class SipBackend { Pjsip, Mock };

    // The mock backend runs as a sandbox: demo account, nothing saved to the user's
    // account or codec settings.
    Application(SipBackend backend, const QString &logFile, QObject *parent = nullptr);

    // startMinimized: start hidden in the tray (autostart), if a tray is available.
    void load(QQmlApplicationEngine &engine, bool startMinimized);

    PhoneController *phone() const { return m_phone; }
    ThemeManager *theme() const { return m_theme; }
    Diagnostics *diagnostics() const { return m_diagnostics; }
    TrayController *tray() const { return m_tray; }

    // Pjsip when the app was built with PJSIP, otherwise Mock.
    static SipBackend defaultBackend();

private:
    // Declaration order is construction order.
    ThemeManager *m_theme;
    WindowSettings *m_windowSettings;
    AutoStart *m_autoStart;
    CallSettings *m_callSettings;
    AccountManager *m_accounts;
    ContactModel *m_contacts;
    ContactFilterModel *m_contactFilter;
    CallHistoryModel *m_history;
    ISipService *m_sip;
    CodecModel *m_codecs;
    Diagnostics *m_diagnostics;
    PhoneController *m_phone;
    TrayController *m_tray;
};
