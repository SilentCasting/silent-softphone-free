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
#include "platform/Permissions.h"
#include "settings/AccountManager.h"
#include "settings/CallSettings.h"
#include "settings/WindowSettings.h"
#include "ui/ThemeManager.h"
#include "ui/TrayController.h"

#ifdef SILENTSOFTPHONE_HAS_PJSIP
#include "sip/PjsipService.h"
#endif

#include <QDateTime>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QStandardPaths>

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

// Contacts and history live next to the other user data, e.g.
// %APPDATA%\SilentSoftphone\SilentSoftphoneFree on Windows.
QString dataFile(Application::SipBackend backend, const QString &name)
{
    if (backend == Application::SipBackend::Mock)
        return {}; // the mock backend is a sandbox: nothing is saved
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(directory);
    return directory + u'/' + name;
}

// Sample data so the mock backend (development, CI screenshots) has something to show.
void seedDemoData(ContactModel *contacts, CallHistoryModel *history)
{
    contacts->addContact({ QStringLiteral("Olena Kovalenko"), QStringLiteral("1002"), QStringLiteral("online") });
    contacts->addContact({ QStringLiteral("Andrii Shevchenko"), QStringLiteral("1003"), QStringLiteral("busy") });
    contacts->addContact({ QStringLiteral("Reception"), QStringLiteral("100"), QStringLiteral("online") });
    contacts->addContact({ QStringLiteral("Support Line"), QStringLiteral("700"), QStringLiteral("away") });
    contacts->addContact({ QStringLiteral("Maria Bondar"), QStringLiteral("+380501112233"), QStringLiteral("offline") });

    const QDateTime now = QDateTime::currentDateTime();
    history->addRecord({ QStringLiteral("Support Line"), QStringLiteral("700"), CallRecord::Kind::Outgoing, now.addDays(-1), 452 });
    history->addRecord({ QString(), QStringLiteral("+380441234567"), CallRecord::Kind::Missed, now.addSecs(-3 * 3600), 0 });
    history->addRecord({ QStringLiteral("Olena Kovalenko"), QStringLiteral("1002"), CallRecord::Kind::Incoming, now.addSecs(-25 * 60), 184 });
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
    , m_permissions(new Permissions(this))
    , m_callSettings(new CallSettings(this))
    , m_accounts(new AccountManager(backend == SipBackend::Mock ? AccountManager::Storage::Demo
                                                                : AccountManager::Storage::Persistent,
                                    this))
    , m_contacts(new ContactModel(dataFile(backend, QStringLiteral("contacts.json")), this))
    , m_contactFilter(new ContactFilterModel(m_contacts, this))
    , m_history(new CallHistoryModel(dataFile(backend, QStringLiteral("history.json")), this))
    , m_sip(createSipService(backend, this))
    , m_codecs(new CodecModel(m_sip, backend != SipBackend::Mock, this))
    , m_diagnostics(new Diagnostics(qobject_cast<MockSipService *>(m_sip), logFile, this))
    , m_phone(new PhoneController(m_sip, m_accounts, m_callSettings, m_contacts, m_history, this))
    , m_tray(new TrayController(m_phone, m_accounts, this))
{
    if (backend == SipBackend::Mock)
        seedDemoData(m_contacts, m_history);
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
    qmlRegisterSingletonInstance(uri, 1, 0, "Permissions", m_permissions);
    qmlRegisterUncreatableType<CallSession>(uri, 1, 0, "CallSession",
                                            QStringLiteral("Calls are created by PhoneController"));

    if (startMinimized && m_tray->isAvailable())
        engine.setInitialProperties({ { QStringLiteral("startHidden"), true } });
    engine.loadFromModule("SilentSoftphone", "Main");

    m_phone->registerAccount();
}

void Application::requestPermissions()
{
    m_permissions->requestMissing();
}
