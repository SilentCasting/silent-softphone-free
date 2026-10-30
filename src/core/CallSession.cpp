// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/CallSession.h"

#include "core/Format.h"
#include "core/SipAccount.h"

CallSession::CallSession(int callId, Direction direction, const QString &remoteUri,
                         const QString &remoteName, QObject *parent)
    : QObject(parent)
    , m_callId(callId)
    , m_direction(direction)
    , m_remoteUri(remoteUri)
    , m_remoteName(remoteName)
    , m_state(direction == Incoming ? Alerting : Dialing)
    , m_startedAt(QDateTime::currentDateTime())
{
    m_ticker.setInterval(1000);
    connect(&m_ticker, &QTimer::timeout, this, [this] {
        m_duration = int(m_answeredAt.secsTo(QDateTime::currentDateTime()));
        emit durationChanged();
    });
}

QString CallSession::number() const
{
    return sipUserPart(m_remoteUri);
}

QString CallSession::displayName() const
{
    return m_remoteName.isEmpty() ? number() : m_remoteName;
}

QString CallSession::stateText() const
{
    switch (m_state) {
    case Dialing:  return tr("Calling…");
    case Ringing:  return tr("Ringing…");
    case Alerting: return tr("Incoming call");
    case Active:   return tr("Connected");
    case Held:     return tr("On hold");
    case Ended:    return tr("Call ended");
    }
    return {};
}

QString CallSession::durationText() const
{
    return formatDuration(m_duration);
}

void CallSession::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;

    if (state == Active && !m_answeredAt.isValid()) {
        m_answeredAt = QDateTime::currentDateTime();
        m_ticker.start();
    } else if (state == Ended) {
        m_ticker.stop();
    }
    emit stateChanged();
}

void CallSession::setMuted(bool muted)
{
    if (m_muted == muted)
        return;
    m_muted = muted;
    emit mutedChanged();
}

void CallSession::setMedia(const CallMediaInfo &media)
{
    m_media = media;
    emit mediaChanged();
}
