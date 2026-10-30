// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/CallSession.h"
#include "core/SipAccount.h"
#include "core/SipTypes.h"

#include <QList>
#include <QObject>
#include <QString>

// Boundary between the application and the SIP/RTP stack. Everything above this
// interface is stack-agnostic, so MockSipService can be replaced by a PJSIP-backed
// implementation without touching controllers or QML.
//
// Implementations must emit signals on the GUI thread.
class ISipService : public QObject
{
    Q_OBJECT

public:
    enum class RegistrationState { Unregistered, Registering, Registered, Failed };
    Q_ENUM(RegistrationState)

    using QObject::QObject;
    ~ISipService() override = default;

    virtual RegistrationState registrationState() const = 0;
    virtual void registerAccount(const SipAccount &account) = 0;
    virtual void unregisterAccount() = 0;

    // Returns the id of the new call, or -1 if it could not be started.
    virtual int makeCall(const QString &uri) = 0;
    virtual void answer(int callId) = 0;
    virtual void hangup(int callId) = 0;
    virtual void setHold(int callId, bool hold) = 0;
    virtual void setMute(int callId, bool mute) = 0;
    virtual void sendDtmf(int callId, const QString &digits, DtmfMethod method) = 0;
    virtual CallMediaInfo callMedia(int callId) const = 0;

    // Local key-press feedback on the speaker; never sent to the remote side.
    virtual void playKeyTone(QChar digit) = 0;

    virtual QList<SipCodec> codecs() const = 0;
    virtual void setCodecPriority(const QString &codecId, int priority) = 0;

signals:
    void registrationStateChanged(ISipService::RegistrationState state, const QString &reason);
    void incomingCall(int callId, const QString &remoteUri, const QString &remoteName);
    void callStateChanged(int callId, CallSession::State state);
};
