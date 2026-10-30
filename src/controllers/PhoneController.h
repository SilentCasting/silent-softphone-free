// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ISipService.h"

#include <QObject>
#include <QPointer>
#include <QTimer>

class AccountManager;
class CallHistoryModel;
class CallSession;
class CallSettings;
class ContactModel;

// Call workflows exposed to QML. Owns the current CallSession, reacts to the SIP
// service and writes finished calls to the history. MVP rule: one call at a time.
class PhoneController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString registrationStatus READ registrationStatus NOTIFY registrationChanged)
    Q_PROPERTY(QString registrationText READ registrationText NOTIFY registrationChanged)
    Q_PROPERTY(CallSession *currentCall READ currentCall NOTIFY currentCallChanged)
    Q_PROPERTY(bool inCall READ inCall NOTIFY currentCallChanged)

public:
    PhoneController(ISipService *sip, AccountManager *accounts, CallSettings *settings,
                    ContactModel *contacts, CallHistoryModel *history, QObject *parent = nullptr);

    // "registered" | "registering" | "failed" | "offline", used by QML for styling.
    QString registrationStatus() const;
    QString registrationText() const;

    CallSession *currentCall() const { return m_call; }
    bool inCall() const { return m_call != nullptr; }

    Q_INVOKABLE void registerAccount();
    Q_INVOKABLE void dial(const QString &number);
    Q_INVOKABLE void answer();
    Q_INVOKABLE void hangup();
    Q_INVOKABLE void toggleHold();
    Q_INVOKABLE void toggleMute();
    // Sends a DTMF digit in an active call, with local key-tone feedback.
    Q_INVOKABLE void sendDtmf(const QString &digit);
    // Key-tone feedback only, e.g. while typing a number.
    Q_INVOKABLE void playKeyTone(const QString &digit);

signals:
    void registrationChanged();
    void currentCallChanged();
    void missedCall(const QString &caller);

private:
    void onRegistrationStateChanged(ISipService::RegistrationState state, const QString &reason);
    void onIncomingCall(int callId, const QString &remoteUri, const QString &remoteName);
    void onCallStateChanged(int callId, CallSession::State state);
    void pollCallMedia();

    QString toSipUri(const QString &target) const;
    void setCurrentCall(CallSession *call);
    void finishCurrentCall();

    ISipService *m_sip;
    AccountManager *m_accounts;
    CallSettings *m_settings;
    ContactModel *m_contacts;
    CallHistoryModel *m_history;

    ISipService::RegistrationState m_registrationState = ISipService::RegistrationState::Unregistered;
    QString m_registrationReason;
    QPointer<CallSession> m_call;
    QTimer m_mediaPoll;
    // Account switched during a call; re-register once the call ends.
    bool m_registrationPending = false;
};
