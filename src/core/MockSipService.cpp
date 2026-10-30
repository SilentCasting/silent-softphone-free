// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/MockSipService.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

#include <iterator>

namespace {

constexpr int kRegisterDelayMs = 900;
constexpr int kRemoteRingDelayMs = 1200;
constexpr int kRemoteAnswerDelayMs = 3500;
constexpr int kLocalAnswerDelayMs = 300;
constexpr int kHoldDelayMs = 200;
constexpr int kMissedTimeoutMs = 30000;

struct DemoCaller
{
    const char *name;
    const char *number;
};

constexpr DemoCaller kDemoCallers[] = {
    { "Olena Kovalenko", "1002" },
    { "Support Line", "700" },
    { "", "+380441234567" },
};

} // namespace

MockSipService::MockSipService(QObject *parent)
    : ISipService(parent)
    , m_codecs{
          { QStringLiteral("G722/16000/1"), 133 },
          { QStringLiteral("PCMA/8000/1"), 132 },
          { QStringLiteral("PCMU/8000/1"), 131 },
          { QStringLiteral("speex/16000/1"), 130 },
          { QStringLiteral("GSM/8000/1"), 128 },
          { QStringLiteral("L16/44100/1"), 0 },
      }
{
}

void MockSipService::registerAccount(const SipAccount &account)
{
    m_domain = account.domain;
    const int attempt = ++m_registrationAttempt;
    const bool valid = account.isValid();

    setRegistrationState(RegistrationState::Registering);
    QTimer::singleShot(kRegisterDelayMs, this, [this, attempt, valid] {
        if (attempt != m_registrationAttempt)
            return;
        if (valid)
            setRegistrationState(RegistrationState::Registered);
        else
            setRegistrationState(RegistrationState::Failed, tr("username and domain are required"));
    });
}

void MockSipService::unregisterAccount()
{
    ++m_registrationAttempt;
    setRegistrationState(RegistrationState::Unregistered);
}

int MockSipService::makeCall(const QString &uri)
{
    if (uri.isEmpty() || m_registrationState != RegistrationState::Registered)
        return -1;

    const int callId = m_nextCallId++;
    m_liveCalls.insert(callId);
    m_answeredCalls.insert(callId);
    emitStateLater(callId, CallSession::Ringing, kRemoteRingDelayMs);
    emitStateLater(callId, CallSession::Active, kRemoteAnswerDelayMs);
    return callId;
}

void MockSipService::answer(int callId)
{
    if (!m_liveCalls.contains(callId))
        return;
    m_answeredCalls.insert(callId);
    emitStateLater(callId, CallSession::Active, kLocalAnswerDelayMs);
}

void MockSipService::hangup(int callId)
{
    if (!m_liveCalls.contains(callId))
        return;
    // Drop the call immediately so pending "remote" transitions are discarded.
    m_liveCalls.remove(callId);
    QTimer::singleShot(0, this, [this, callId] { endCall(callId); });
}

void MockSipService::setHold(int callId, bool hold)
{
    emitStateLater(callId, hold ? CallSession::Held : CallSession::Active, kHoldDelayMs);
}

void MockSipService::setMute(int callId, bool mute)
{
    qDebug() << "[mock-sip] call" << callId << (mute ? "muted" : "unmuted");
}

void MockSipService::sendDtmf(int callId, const QString &digits, DtmfMethod method)
{
    qDebug() << "[mock-sip] call" << callId << "DTMF" << digits << "method" << int(method);
}

CallMediaInfo MockSipService::callMedia(int callId) const
{
    if (!m_liveCalls.contains(callId))
        return {};
    // Speech-like flicker so the level meters have something to show.
    auto *random = QRandomGenerator::global();
    return { 0.05 + random->bounded(0.5), 0.05 + random->bounded(0.5), QStringLiteral("PCMA 8 kHz") };
}

void MockSipService::playKeyTone(QChar digit)
{
    qDebug() << "[mock-sip] key tone" << digit;
}

void MockSipService::setCodecPriority(const QString &codecId, int priority)
{
    for (SipCodec &codec : m_codecs) {
        if (codec.id == codecId)
            codec.priority = priority;
    }
}

void MockSipService::simulateIncomingCall()
{
    const auto &caller = kDemoCallers[QRandomGenerator::global()->bounded(int(std::size(kDemoCallers)))];
    const QString number = QString::fromLatin1(caller.number);
    const QString uri = m_domain.isEmpty()
        ? QStringLiteral("sip:%1").arg(number)
        : QStringLiteral("sip:%1@%2").arg(number, m_domain);

    const int callId = m_nextCallId++;
    m_liveCalls.insert(callId);
    emit incomingCall(callId, uri, QString::fromUtf8(caller.name));

    QTimer::singleShot(kMissedTimeoutMs, this, [this, callId] {
        if (m_liveCalls.contains(callId) && !m_answeredCalls.contains(callId)) {
            m_liveCalls.remove(callId);
            endCall(callId);
        }
    });
}

void MockSipService::setRegistrationState(RegistrationState state, const QString &reason)
{
    if (m_registrationState == state && reason.isEmpty())
        return;
    m_registrationState = state;
    emit registrationStateChanged(state, reason);
}

void MockSipService::emitStateLater(int callId, CallSession::State state, int delayMs)
{
    QTimer::singleShot(delayMs, this, [this, callId, state] {
        if (m_liveCalls.contains(callId))
            emit callStateChanged(callId, state);
    });
}

void MockSipService::endCall(int callId)
{
    m_answeredCalls.remove(callId);
    emit callStateChanged(callId, CallSession::Ended);
}
