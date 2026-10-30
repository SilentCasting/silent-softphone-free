// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controllers/PhoneController.h"

#include "core/CallSession.h"
#include "models/CallHistoryModel.h"
#include "models/ContactModel.h"
#include "settings/AccountManager.h"
#include "settings/CallSettings.h"

namespace {

constexpr int kMediaPollIntervalMs = 100;

bool isDtmfDigit(const QString &digit)
{
    return digit.size() == 1 && QStringLiteral("0123456789*#ABCD").contains(digit.at(0));
}

} // namespace

PhoneController::PhoneController(ISipService *sip, AccountManager *accounts, CallSettings *settings,
                                 ContactModel *contacts, CallHistoryModel *history, QObject *parent)
    : QObject(parent)
    , m_sip(sip)
    , m_accounts(accounts)
    , m_settings(settings)
    , m_contacts(contacts)
    , m_history(history)
    , m_registrationState(sip->registrationState())
{
    connect(m_sip, &ISipService::registrationStateChanged, this, &PhoneController::onRegistrationStateChanged);
    connect(m_sip, &ISipService::incomingCall, this, &PhoneController::onIncomingCall);
    connect(m_sip, &ISipService::callStateChanged, this, &PhoneController::onCallStateChanged);
    connect(m_accounts, &AccountManager::activeAccountChanged, this, &PhoneController::registerAccount);
    // The registered address is part of the status text.
    connect(m_accounts, &AccountManager::currentChanged, this, &PhoneController::registrationChanged);

    m_mediaPoll.setInterval(kMediaPollIntervalMs);
    connect(&m_mediaPoll, &QTimer::timeout, this, &PhoneController::pollCallMedia);
}

QString PhoneController::registrationStatus() const
{
    using State = ISipService::RegistrationState;
    switch (m_registrationState) {
    case State::Registered:   return QStringLiteral("registered");
    case State::Registering:  return QStringLiteral("registering");
    case State::Failed:       return QStringLiteral("failed");
    case State::Unregistered: return QStringLiteral("offline");
    }
    return {};
}

QString PhoneController::registrationText() const
{
    using State = ISipService::RegistrationState;
    switch (m_registrationState) {
    case State::Registered:   return tr("Registered · %1").arg(m_accounts->uri());
    case State::Registering:  return tr("Registering…");
    case State::Failed:       return m_registrationReason.isEmpty()
                                  ? tr("Registration failed")
                                  : tr("Registration failed: %1").arg(m_registrationReason);
    case State::Unregistered: return m_accounts->count() == 0 ? tr("No account — add one in Settings")
                                                               : tr("Offline");
    }
    return {};
}

void PhoneController::registerAccount()
{
    if (m_call) {
        // Re-registering drops the account the call belongs to.
        m_registrationPending = true;
        return;
    }
    m_registrationPending = false;

    const SipAccount account = m_accounts->current();
    if (account.isValid())
        m_sip->registerAccount(account);
    else
        m_sip->unregisterAccount();
}

void PhoneController::dial(const QString &number)
{
    const QString target = number.trimmed();
    if (target.isEmpty() || m_call)
        return;

    const QString uri = toSipUri(target);
    const int callId = m_sip->makeCall(uri);
    if (callId < 0)
        return;

    setCurrentCall(new CallSession(callId, CallSession::Outgoing, uri,
                                   m_contacts->nameForNumber(sipUserPart(uri)), this));
}

void PhoneController::answer()
{
    if (m_call && m_call->state() == CallSession::Alerting)
        m_sip->answer(m_call->callId());
}

void PhoneController::hangup()
{
    if (m_call)
        m_sip->hangup(m_call->callId());
}

void PhoneController::toggleHold()
{
    if (!m_call)
        return;
    const auto state = m_call->state();
    if (state == CallSession::Active || state == CallSession::Held)
        m_sip->setHold(m_call->callId(), state == CallSession::Active);
}

void PhoneController::toggleMute()
{
    if (!m_call)
        return;
    const bool muted = !m_call->isMuted();
    m_sip->setMute(m_call->callId(), muted);
    m_call->setMuted(muted);
}

void PhoneController::sendDtmf(const QString &digit)
{
    if (!m_call || m_call->state() != CallSession::Active || !isDtmfDigit(digit))
        return;
    playKeyTone(digit);
    m_sip->sendDtmf(m_call->callId(), digit, m_settings->dtmfMethod());
}

void PhoneController::playKeyTone(const QString &digit)
{
    if (m_settings->keyTones() && isDtmfDigit(digit))
        m_sip->playKeyTone(digit.at(0));
}

void PhoneController::onRegistrationStateChanged(ISipService::RegistrationState state, const QString &reason)
{
    m_registrationState = state;
    m_registrationReason = reason;
    emit registrationChanged();
}

void PhoneController::onIncomingCall(int callId, const QString &remoteUri, const QString &remoteName)
{
    if (m_call) {
        // Single-line MVP: reject while busy.
        m_sip->hangup(callId);
        return;
    }
    const QString name = remoteName.isEmpty() ? m_contacts->nameForNumber(sipUserPart(remoteUri)) : remoteName;
    setCurrentCall(new CallSession(callId, CallSession::Incoming, remoteUri, name, this));
}

void PhoneController::onCallStateChanged(int callId, CallSession::State state)
{
    if (!m_call || m_call->callId() != callId)
        return;
    m_call->setState(state);
    if (state == CallSession::Ended)
        finishCurrentCall();
}

void PhoneController::pollCallMedia()
{
    if (!m_call)
        return;
    const auto state = m_call->state();
    if (state == CallSession::Active || state == CallSession::Held)
        m_call->setMedia(m_sip->callMedia(m_call->callId()));
}

QString PhoneController::toSipUri(const QString &target) const
{
    if (target.startsWith(QLatin1String("sip:")) || target.startsWith(QLatin1String("sips:")))
        return target;
    const QString domain = m_accounts->current().domain;
    if (target.contains(u'@') || domain.isEmpty())
        return QStringLiteral("sip:") + target;
    return QStringLiteral("sip:%1@%2").arg(target, domain);
}

void PhoneController::setCurrentCall(CallSession *call)
{
    m_call = call;
    if (call)
        m_mediaPoll.start();
    else
        m_mediaPoll.stop();
    emit currentCallChanged();
}

void PhoneController::finishCurrentCall()
{
    CallSession *call = m_call;

    CallRecord record;
    record.name = call->remoteName();
    record.number = call->number();
    record.startedAt = call->startedAt();
    record.durationSec = call->duration();
    if (call->direction() == CallSession::Outgoing)
        record.kind = CallRecord::Kind::Outgoing;
    else
        record.kind = call->wasAnswered() ? CallRecord::Kind::Incoming : CallRecord::Kind::Missed;
    m_history->addRecord(record);
    if (record.kind == CallRecord::Kind::Missed)
        emit missedCall(call->displayName());

    setCurrentCall(nullptr);
    call->deleteLater();

    if (m_registrationPending)
        registerAccount();
}
