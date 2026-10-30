// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/Application.h"

#include "app/AutoStart.h"
#include "app/Diagnostics.h"
#include "controllers/PhoneController.h"
#include "core/CallSession.h"
#include "core/MockSipService.h"
#include "models/CallHistoryModel.h"
#include "models/CodecModel.h"
#include "models/ContactModel.h"
#include "settings/AccountManager.h"
#include "settings/CallSettings.h"
#include "settings/WindowSettings.h"
#include "ui/ThemeManager.h"
#include "ui/TrayController.h"

#ifdef SILENTSOFTPHONE_HAS_PJSIP
#include "sip/PjsipService.h"
#endif

#include <QQmlApplicationEngine>
#include <QQmlEngine>

namespace {

ISipService *createSipService(Application::SipBackend backend, QObject *parent)
{
#ifdef SILENTSOFTPHONE_HAS_PJSIP
    if (backend == Application::SipBackend::Pjsip)
        return new PjsipService(parent);
#else
    Q_UNUSED(backend)
#endif
    return new MockSipService(parent);
}

} // namespace

Application::SipBackend Application::defaultBackend()
{
#ifdef SILENTSOFTPHONE_HAS_PJSIP
    return SipBackend::Pjsip;
#else
    return SipBackend::Mock;
#endif
}

Application::Application(SipBackend backend, const QString &logFile, QObject *parent)
    : QObject(parent)
    , m_theme(new ThemeManager(this))
    , m_windowSettings(new WindowSettings(this))
    , m_autoStart(new AutoStart(this))
    , m_callSettings(new CallSettings(this))
    , m_accounts(new AccountManager(backend == SipBackend::Mock ? AccountManager::Storage::Demo
                                                                : AccountManager::Storage::Persistent,
                                    this))
    , m_contacts(new ContactModel(this))
    , m_contactFilter(new ContactFilterModel(m_contacts, this))
    , m_history(new CallHistoryModel(this))
    , m_sip(createSipService(backend, this))
    , m_codecs(new CodecModel(m_sip, backend != SipBackend::Mock, this))
    , m_diagnostics(new Diagnostics(qobject_cast<MockSipService *>(m_sip), logFile, this))
    , m_phone(new PhoneController(m_sip, m_accounts, m_callSettings, m_contacts, m_history, this))
    , m_tray(new TrayController(m_phone, m_accounts, this))
{
}

void Application::load(QQmlApplicationEngine &engine, bool startMinimized)
{
    constexpr auto uri = "SilentSoftphone.Core";
    qmlRegisterSingletonInstance(uri, 1, 0, "Theme", m_theme);
    qmlRegisterSingletonInstance(uri, 1, 0, "Accounts", m_accounts);
    qmlRegisterSingletonInstance(uri, 1, 0, "CallSettings", m_callSettings);
    qmlRegisterSingletonInstance(uri, 1, 0, "Codecs", m_codecs);
    qmlRegisterSingletonInstance(uri, 1, 0, "Contacts", m_contactFilter);
    qmlRegisterSingletonInstance(uri, 1, 0, "History", m_history);
    qmlRegisterSingletonInstance(uri, 1, 0, "Phone", m_phone);
    qmlRegisterSingletonInstance(uri, 1, 0, "Diagnostics", m_diagnostics);
    qmlRegisterSingletonInstance(uri, 1, 0, "Tray", m_tray);
    qmlRegisterSingletonInstance(uri, 1, 0, "WindowSettings", m_windowSettings);
    qmlRegisterSingletonInstance(uri, 1, 0, "AutoStart", m_autoStart);
    qmlRegisterUncreatableType<CallSession>(uri, 1, 0, "CallSession",
                                            QStringLiteral("Calls are created by PhoneController"));

    if (startMinimized && m_tray->isAvailable())
        engine.setInitialProperties({ { QStringLiteral("startHidden"), true } });
    engine.loadFromModule("SilentSoftphone", "Main");

    m_phone->registerAccount();
}
