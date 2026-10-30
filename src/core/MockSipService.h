// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ISipService.h"

#include <QSet>

// In-process stand-in for a PBX: registers after a short delay, "answers" outgoing
// calls after a few seconds and can fake incoming calls for UI development.
class MockSipService final : public ISipService
{
    Q_OBJECT

public:
    explicit MockSipService(QObject *parent = nullptr);

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

    QList<SipCodec> codecs() const override { return m_codecs; }
    void setCodecPriority(const QString &codecId, int priority) override;

    Q_INVOKABLE void simulateIncomingCall();

private:
    void setRegistrationState(RegistrationState state, const QString &reason = {});
    void emitStateLater(int callId, CallSession::State state, int delayMs);
    void endCall(int callId);

    RegistrationState m_registrationState = RegistrationState::Unregistered;
    int m_registrationAttempt = 0;
    QString m_domain;
    int m_nextCallId = 1;
    QSet<int> m_liveCalls;
    QSet<int> m_answeredCalls;
    QList<SipCodec> m_codecs;
};
