// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ISipService.h"

#include <memory>

// ISipService on top of PJSIP's pjsua2 API.
//
// PJSIP runs without its own worker threads: a timer polls the endpoint on the GUI
// thread, so every pjsua2 callback (and every signal) is delivered on the GUI thread.
// pjsua2 headers stay inside the .cpp to keep them out of the rest of the app.
class PjsipService final : public ISipService
{
    Q_OBJECT

public:
    explicit PjsipService(QObject *parent = nullptr);
    ~PjsipService() override;

    bool isReady() const;
    // One-line summary of the PJSIP version and transports, for diagnostics.
    QString diagnostics() const;

    RegistrationState registrationState() const override { return m_registrationState; }
    void registerAccount(const SipAccount &account) override;
    void unregisterAccount() override;

    int makeCall(const QString &uri) override;
    void answer(int callId) override;
    void hangup(int callId) override;
    void setHold(int callId, bool hold) override;
    void setMute(int callId, bool mute) override;
    void sendDtmf(int callId, const QString &digits, DtmfMethod method) override;
    CallMediaInfo callMedia(int callId) const override;

    void playKeyTone(QChar digit) override;

    QList<SipCodec> codecs() const override;
    void setCodecPriority(const QString &codecId, int priority) override;

private:
    class PjAccount;
    class PjCall;
    struct Private;

    void setRegistrationState(RegistrationState state, const QString &reason = {});
    void postCallState(int callId, CallSession::State state);
    void handleIncomingCall(PjAccount &account, int callId);
    void releaseCall(PjCall *call);
    PjCall *findCall(int callId) const;
    void releaseKeyTonesWhenIdle();

    std::unique_ptr<Private> d;
    RegistrationState m_registrationState = RegistrationState::Unregistered;
};
