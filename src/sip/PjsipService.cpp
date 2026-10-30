// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// pjsua2 first: on Windows it must pull in winsock2.h before anything includes windows.h.
#include <pjsua2.hpp>

#include "sip/PjsipService.h"

#include <QCoreApplication>
#include <QHash>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QTimer>

Q_LOGGING_CATEGORY(lcSip, "silentsoftphone.sip")

namespace {

constexpr int kPollIntervalMs = 10;
constexpr int kKeyToneOnMs = 120;
constexpr int kKeyToneOffMs = 20;
constexpr short kKeyToneVolume = 8000; // PJMEDIA default is noticeably loud for UI feedback
constexpr int kKeyToneIdleCheckMs = 250;

QString fromStd(const std::string &value)
{
    return QString::fromStdString(value);
}

std::string toStd(const QString &value)
{
    return value.toStdString();
}

// Forwards PJSIP's log to Qt logging (category "silentsoftphone.sip").
class QtLogWriter final : public pj::LogWriter
{
public:
    void write(const pj::LogEntry &entry) override
    {
        const QString message = fromStd(entry.msg).trimmed();
        if (entry.level <= 1)
            qCWarning(lcSip).noquote() << message;
        else
            qCDebug(lcSip).noquote() << message;
    }
};

// "\"Olena\" <sip:1002@pbx>;tag=x" -> { "Olena", "sip:1002@pbx" }
std::pair<QString, QString> parseNameAddr(const QString &value)
{
    static const QRegularExpression nameAddr(QStringLiteral(R"re(^\s*(?:"([^"]*)"|([^<]*?))\s*<([^>]+)>)re"));
    const QRegularExpressionMatch match = nameAddr.match(value);
    if (!match.hasMatch())
        return { QString(), value.trimmed() };
    const QString name = match.captured(1).isEmpty() ? match.captured(2).trimmed() : match.captured(1);
    return { name, match.captured(3) };
}

QString transportParam(SipAccount::Transport transport)
{
    switch (transport) {
    case SipAccount::Transport::Udp: return {};
    case SipAccount::Transport::Tcp: return QStringLiteral(";transport=tcp");
    case SipAccount::Transport::Tls: return QStringLiteral(";transport=tls");
    }
    return {};
}

} // namespace

class PjsipService::PjCall final : public pj::Call
{
public:
    PjCall(PjsipService &service, pj::Account &account, int callId = PJSUA_INVALID_ID)
        : pj::Call(account, callId)
        , m_service(service)
    {
    }

    void onCallState(pj::OnCallStateParam &) override
    {
        const pj::CallInfo info = getInfo();
        switch (info.state) {
        case PJSIP_INV_STATE_CALLING:
            m_service.postCallState(info.id, CallSession::Dialing);
            break;
        case PJSIP_INV_STATE_EARLY:
            if (info.role == PJSIP_ROLE_UAC)
                m_service.postCallState(info.id, CallSession::Ringing);
            break;
        case PJSIP_INV_STATE_CONFIRMED:
            m_service.postCallState(info.id, CallSession::Active);
            break;
        case PJSIP_INV_STATE_DISCONNECTED:
            qCInfo(lcSip) << "call" << info.id << "ended:" << info.lastStatusCode << fromStd(info.lastReason);
            m_service.postCallState(info.id, CallSession::Ended);
            m_service.releaseCall(this);
            break;
        default:
            break;
        }
    }

    void onCallMediaState(pj::OnCallMediaStateParam &) override
    {
        const pj::CallInfo info = getInfo();
        bool localHold = false;
        for (unsigned i = 0; i < info.media.size(); ++i) {
            const pj::CallMediaInfo &media = info.media[i];
            if (media.type != PJMEDIA_TYPE_AUDIO)
                continue;
            if (media.status == PJSUA_CALL_MEDIA_ACTIVE || media.status == PJSUA_CALL_MEDIA_REMOTE_HOLD)
                connectAudio(int(i));
            else if (media.status == PJSUA_CALL_MEDIA_LOCAL_HOLD)
                localHold = true;
        }
        if (info.state == PJSIP_INV_STATE_CONFIRMED)
            m_service.postCallState(info.id, localHold ? CallSession::Held : CallSession::Active);
    }

    CallMediaInfo media() const
    {
        CallMediaInfo info;
        info.codec = m_codec;
        if (m_audioIndex < 0)
            return info;
        try {
            const pj::AudioMedia audio = getAudioMedia(m_audioIndex);
            // pjsua2 names levels from the port's side: "rx" is bridge -> call (what we
            // send), "tx" is call -> bridge (what the remote side sends us). 0..100.
            info.sendLevel = audio.getRxLevel() / 100.0;
            info.receiveLevel = audio.getTxLevel() / 100.0;
        } catch (const pj::Error &) {
        }
        return info;
    }

    void setMuted(bool muted)
    {
        m_muted = muted;
        if (m_audioIndex < 0)
            return;
        try {
            pj::AudioMedia media = getAudioMedia(m_audioIndex);
            pj::AudioMedia &capture = pj::Endpoint::instance().audDevManager().getCaptureDevMedia();
            if (muted)
                capture.stopTransmit(media);
            else
                capture.startTransmit(media);
        } catch (const pj::Error &error) {
            qCWarning(lcSip) << "mute failed:" << fromStd(error.info());
        }
    }

private:
    void connectAudio(int index)
    {
        try {
            pj::AudioMedia media = getAudioMedia(index);
            pj::AudDevManager &devices = pj::Endpoint::instance().audDevManager();
            media.startTransmit(devices.getPlaybackDevMedia());
            if (!m_muted)
                devices.getCaptureDevMedia().startTransmit(media);
            m_audioIndex = index;

            const pj::StreamInfo stream = getStreamInfo(unsigned(index));
            m_codec = QStringLiteral("%1 %2 kHz").arg(fromStd(stream.codecName)).arg(stream.codecClockRate / 1000.0);
            qCInfo(lcSip) << "audio connected, codec" << m_codec;
        } catch (const pj::Error &error) {
            qCWarning(lcSip) << "audio connection failed:" << fromStd(error.info());
        }
    }

    PjsipService &m_service;
    bool m_muted = false;
    int m_audioIndex = -1;
    QString m_codec;
};

class PjsipService::PjAccount final : public pj::Account
{
public:
    explicit PjAccount(PjsipService &service)
        : m_service(service)
    {
    }

    void onRegState(pj::OnRegStateParam &param) override
    {
        const pj::AccountInfo info = getInfo();
        if (info.regIsActive)
            m_service.setRegistrationState(RegistrationState::Registered);
        else if (param.code / 100 == 2)
            m_service.setRegistrationState(RegistrationState::Unregistered);
        else
            m_service.setRegistrationState(RegistrationState::Failed,
                                           QStringLiteral("%1 %2").arg(int(param.code)).arg(fromStd(param.reason)));
    }

    void onIncomingCall(pj::OnIncomingCallParam &param) override
    {
        m_service.handleIncomingCall(*this, param.callId);
    }

private:
    PjsipService &m_service;
};

struct PjsipService::Private
{
    pj::Endpoint endpoint;
    std::unique_ptr<PjAccount> account;
    QHash<int, PjCall *> calls;
    // Connected to the speaker only while playing, so the sound device can close when idle.
    std::unique_ptr<pj::ToneGenerator> keyTones;
    bool keyTonesConnected = false;
    QTimer keyToneIdleCheck;
    QTimer poller;
    bool ready = false;
    int udpTransport = -1;
    int tcpTransport = -1;
    int tlsTransport = -1;
};

PjsipService::PjsipService(QObject *parent)
    : ISipService(parent)
    , d(std::make_unique<Private>())
{
    try {
        d->endpoint.libCreate();

        pj::EpConfig config;
        config.uaConfig.userAgent = toStd(QStringLiteral("SilentSoftphoneFree/%1").arg(QCoreApplication::applicationVersion()));
        config.uaConfig.threadCnt = 0; // callbacks come from poll() on the GUI thread
        config.logConfig.level = 5;
        config.logConfig.consoleLevel = 4;
        config.logConfig.decor &= ~unsigned(PJ_LOG_HAS_NEWLINE);
        config.logConfig.writer = new QtLogWriter; // owned by the endpoint
        d->endpoint.libInit(config);

        pj::TransportConfig transport;
        transport.port = 0; // any free port, so we don't clash with other softphones
        d->udpTransport = d->endpoint.transportCreate(PJSIP_TRANSPORT_UDP, transport);
        const auto optionalTransport = [&](pjsip_transport_type_e type, const char *name) {
            try {
                return d->endpoint.transportCreate(type, transport);
            } catch (const pj::Error &error) {
                qCWarning(lcSip) << name << "transport unavailable:" << fromStd(error.info());
                return -1;
            }
        };
        d->tcpTransport = optionalTransport(PJSIP_TRANSPORT_TCP, "TCP");
        d->tlsTransport = optionalTransport(PJSIP_TRANSPORT_TLS, "TLS");

        d->endpoint.libStart();
        d->ready = true;
    } catch (const pj::Error &error) {
        qCCritical(lcSip) << "PJSIP initialisation failed:" << fromStd(error.info());
    }

    connect(&d->poller, &QTimer::timeout, this, [this] { d->endpoint.libHandleEvents(0); });
    if (d->ready)
        d->poller.start(kPollIntervalMs);

    d->keyToneIdleCheck.setInterval(kKeyToneIdleCheckMs);
    connect(&d->keyToneIdleCheck, &QTimer::timeout, this, &PjsipService::releaseKeyTonesWhenIdle);
}

PjsipService::~PjsipService()
{
    d->poller.stop();
    if (d->ready) {
        try {
            d->endpoint.hangupAllCalls();
        } catch (const pj::Error &) {
        }
    }
    d->keyToneIdleCheck.stop();
    d->keyTones.reset(); // media ports must go before libDestroy()
    qDeleteAll(d->calls);
    d->calls.clear();
    d->account.reset();
    try {
        d->endpoint.libDestroy();
    } catch (const pj::Error &) {
    }
}

bool PjsipService::isReady() const
{
    return d->ready;
}

QString PjsipService::diagnostics() const
{
    if (!d->ready)
        return QStringLiteral("PJSIP not initialised");
    const auto state = [](int id) { return id >= 0 ? QStringLiteral("yes") : QStringLiteral("no"); };
    return QStringLiteral("PJSIP %1, transports: UDP %2, TCP %3, TLS %4")
        .arg(fromStd(d->endpoint.libVersion().full), state(d->udpTransport),
             state(d->tcpTransport), state(d->tlsTransport));
}

void PjsipService::registerAccount(const SipAccount &account)
{
    if (!d->ready) {
        setRegistrationState(RegistrationState::Failed, tr("SIP stack is not initialised"));
        return;
    }

    const int transportId = account.transport == SipAccount::Transport::Tls ? d->tlsTransport
                          : account.transport == SipAccount::Transport::Tcp ? d->tcpTransport
                          : d->udpTransport;
    if (transportId < 0) {
        setRegistrationState(RegistrationState::Failed, tr("transport is not available"));
        return;
    }

    d->account.reset(); // drops the previous registration
    setRegistrationState(RegistrationState::Registering);

    pj::AccountConfig config;
    const QString aor = account.uri();
    const QString displayName = QString(account.displayName).replace(u'"', QStringLiteral("\\\""));
    config.idUri = toStd(displayName.isEmpty() ? aor : QStringLiteral("\"%1\" <%2>").arg(displayName, aor));
    config.regConfig.registrarUri = toStd(QStringLiteral("sip:%1%2").arg(account.domain, transportParam(account.transport)));
    config.sipConfig.authCreds.push_back(
        pj::AuthCredInfo("digest", "*", toStd(account.username), 0, toStd(account.password)));
    config.sipConfig.transportId = transportId;

    try {
        auto pjAccount = std::make_unique<PjAccount>(*this);
        pjAccount->create(config);
        d->account = std::move(pjAccount);
    } catch (const pj::Error &error) {
        setRegistrationState(RegistrationState::Failed, fromStd(error.reason));
    }
}

void PjsipService::unregisterAccount()
{
    // Deleting the pjsua account sends the un-REGISTER and drops it, so a stale
    // account can't be used for calls afterwards.
    d->account.reset();
    setRegistrationState(RegistrationState::Unregistered);
}

int PjsipService::makeCall(const QString &uri)
{
    if (!d->account)
        return -1;

    auto call = std::make_unique<PjCall>(*this, *d->account);
    try {
        pj::CallOpParam param(true);
        call->makeCall(toStd(uri), param);
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "call to" << uri << "failed:" << fromStd(error.info());
        return -1;
    }
    const int callId = call->getId();
    d->calls.insert(callId, call.release());
    return callId;
}

void PjsipService::answer(int callId)
{
    PjCall *call = findCall(callId);
    if (!call)
        return;
    try {
        pj::CallOpParam param;
        param.statusCode = PJSIP_SC_OK;
        call->answer(param);
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "answer failed:" << fromStd(error.info());
    }
}

void PjsipService::hangup(int callId)
{
    PjCall *call = findCall(callId);
    if (!call)
        return;
    try {
        pj::CallOpParam param;
        const pj::CallInfo info = call->getInfo();
        if (info.role == PJSIP_ROLE_UAS && info.state < PJSIP_INV_STATE_CONNECTING)
            param.statusCode = PJSIP_SC_DECLINE; // rejecting a ringing incoming call
        call->hangup(param);
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "hangup failed:" << fromStd(error.info());
    }
}

void PjsipService::setHold(int callId, bool hold)
{
    PjCall *call = findCall(callId);
    if (!call)
        return;
    try {
        pj::CallOpParam param(true);
        if (hold) {
            call->setHold(param);
        } else {
            param.opt.flag = PJSUA_CALL_UNHOLD;
            call->reinvite(param);
        }
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "hold/resume failed:" << fromStd(error.info());
    }
}

void PjsipService::setMute(int callId, bool mute)
{
    if (PjCall *call = findCall(callId))
        call->setMuted(mute);
}

void PjsipService::sendDtmf(int callId, const QString &digits, DtmfMethod method)
{
    PjCall *call = findCall(callId);
    if (!call)
        return;

    const auto send = [&](pjsua_dtmf_method pjMethod, const char *name) {
        pj::CallSendDtmfParam param;
        param.method = pjMethod;
        param.digits = toStd(digits);
        call->sendDtmf(param);
        qCInfo(lcSip) << "DTMF" << digits << "sent via" << name;
    };

    try {
        if (method == DtmfMethod::SipInfo)
            send(PJSUA_DTMF_METHOD_SIP_INFO, "SIP INFO");
        else
            send(PJSUA_DTMF_METHOD_RFC2833, "RFC 2833");
        return;
    } catch (const pj::Error &error) {
        if (method != DtmfMethod::Auto) {
            qCWarning(lcSip) << "DTMF failed:" << fromStd(error.info());
            return;
        }
        // Typically the remote SDP has no telephone-event, so RFC 2833 isn't possible.
        qCInfo(lcSip) << "RFC 2833 DTMF unavailable, falling back to SIP INFO:" << fromStd(error.info());
    }

    try {
        send(PJSUA_DTMF_METHOD_SIP_INFO, "SIP INFO");
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "DTMF failed:" << fromStd(error.info());
    }
}

CallMediaInfo PjsipService::callMedia(int callId) const
{
    const PjCall *call = findCall(callId);
    return call ? call->media() : CallMediaInfo{};
}

void PjsipService::playKeyTone(QChar digit)
{
    if (!d->ready)
        return;
    try {
        if (!d->keyTones) {
            auto tones = std::make_unique<pj::ToneGenerator>();
            tones->createToneGenerator();
            d->keyTones = std::move(tones);
        }
        if (!d->keyTonesConnected) {
            d->keyTones->startTransmit(d->endpoint.audDevManager().getPlaybackDevMedia());
            d->keyTonesConnected = true;
        }

        pj::ToneDigit tone;
        tone.digit = digit.toLatin1();
        tone.on_msec = kKeyToneOnMs;
        tone.off_msec = kKeyToneOffMs;
        tone.volume = kKeyToneVolume;
        d->keyTones->playDigits(pj::ToneDigitVector{ tone });
        d->keyToneIdleCheck.start();
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "key tone failed:" << fromStd(error.info());
    }
}

void PjsipService::releaseKeyTonesWhenIdle()
{
    if (!d->keyTones || d->keyTones->isBusy())
        return;
    d->keyToneIdleCheck.stop();
    if (!d->keyTonesConnected)
        return;
    try {
        d->keyTones->stopTransmit(d->endpoint.audDevManager().getPlaybackDevMedia());
    } catch (const pj::Error &) {
    }
    d->keyTonesConnected = false;
}

QList<SipCodec> PjsipService::codecs() const
{
    QList<SipCodec> result;
    if (!d->ready)
        return result;
    try {
        for (const pj::CodecInfo &codec : d->endpoint.codecEnum2())
            result.append({ fromStd(codec.codecId), int(codec.priority) });
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "codec list failed:" << fromStd(error.info());
    }
    return result;
}

void PjsipService::setCodecPriority(const QString &codecId, int priority)
{
    if (!d->ready)
        return;
    try {
        d->endpoint.codecSetPriority(toStd(codecId), pj_uint8_t(qBound(0, priority, 255)));
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "codec priority failed for" << codecId << fromStd(error.info());
    }
}

void PjsipService::setRegistrationState(RegistrationState state, const QString &reason)
{
    if (m_registrationState == state && reason.isEmpty())
        return;
    m_registrationState = state;
    emit registrationStateChanged(state, reason);
}

void PjsipService::postCallState(int callId, CallSession::State state)
{
    // Queued, so a state change raised from inside makeCall() reaches PhoneController
    // only after it has created the CallSession for the returned id.
    QMetaObject::invokeMethod(this, [this, callId, state] { emit callStateChanged(callId, state); },
                              Qt::QueuedConnection);
}

void PjsipService::handleIncomingCall(PjAccount &account, int callId)
{
    auto *call = new PjCall(*this, account, callId);
    d->calls.insert(callId, call);

    const pj::CallInfo info = call->getInfo();
    try {
        pj::CallOpParam ringing;
        ringing.statusCode = PJSIP_SC_RINGING;
        call->answer(ringing);
    } catch (const pj::Error &error) {
        qCWarning(lcSip) << "180 Ringing failed:" << fromStd(error.info());
    }

    const auto [name, uri] = parseNameAddr(fromStd(info.remoteUri));
    emit incomingCall(callId, uri, name);
}

void PjsipService::releaseCall(PjCall *call)
{
    // pjsua2 calls must be deleted after DISCONNECTED, but not from inside their callback.
    QTimer::singleShot(0, this, [this, call] {
        for (auto it = d->calls.begin(); it != d->calls.end(); ++it) {
            if (it.value() == call) {
                d->calls.erase(it);
                break;
            }
        }
        delete call;
    });
}

PjsipService::PjCall *PjsipService::findCall(int callId) const
{
    return d->calls.value(callId, nullptr);
}
