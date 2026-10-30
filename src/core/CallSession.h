// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/SipTypes.h"

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

// A single call as seen by the UI. State transitions are driven by the SIP service
// through PhoneController; the session itself only tracks timing and local flags.
class CallSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int callId READ callId CONSTANT)
    Q_PROPERTY(Direction direction READ direction CONSTANT)
    Q_PROPERTY(QString remoteUri READ remoteUri CONSTANT)
    Q_PROPERTY(QString number READ number CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(bool onHold READ isOnHold NOTIFY stateChanged)
    Q_PROPERTY(bool muted READ isMuted NOTIFY mutedChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString durationText READ durationText NOTIFY durationChanged)
    Q_PROPERTY(qreal sendLevel READ sendLevel NOTIFY mediaChanged)
    Q_PROPERTY(qreal receiveLevel READ receiveLevel NOTIFY mediaChanged)
    Q_PROPERTY(QString codec READ codec NOTIFY mediaChanged)

public:
    enum Direction { Outgoing, Incoming };
    Q_ENUM(Direction)

    enum State {
        Dialing,   // outgoing, INVITE sent
        Ringing,   // outgoing, remote side is ringing
        Alerting,  // incoming, waiting for the user to answer
        Active,
        Held,
        Ended
    };
    Q_ENUM(State)

    CallSession(int callId, Direction direction, const QString &remoteUri,
                const QString &remoteName, QObject *parent = nullptr);

    int callId() const { return m_callId; }
    Direction direction() const { return m_direction; }
    QString remoteUri() const { return m_remoteUri; }
    QString remoteName() const { return m_remoteName; }
    QString number() const;
    QString displayName() const;

    State state() const { return m_state; }
    QString stateText() const;
    bool isOnHold() const { return m_state == Held; }
    bool isMuted() const { return m_muted; }
    bool wasAnswered() const { return m_answeredAt.isValid(); }

    QDateTime startedAt() const { return m_startedAt; }
    int duration() const { return m_duration; }
    QString durationText() const;

    qreal sendLevel() const { return m_media.sendLevel; }
    qreal receiveLevel() const { return m_media.receiveLevel; }
    QString codec() const { return m_media.codec; }

    void setState(State state);
    void setMuted(bool muted);
    void setMedia(const CallMediaInfo &media);

signals:
    void stateChanged();
    void mutedChanged();
    void durationChanged();
    void mediaChanged();

private:
    const int m_callId;
    const Direction m_direction;
    const QString m_remoteUri;
    const QString m_remoteName;

    State m_state;
    bool m_muted = false;
    QDateTime m_startedAt;
    QDateTime m_answeredAt;
    int m_duration = 0;
    QTimer m_ticker;
    CallMediaInfo m_media;
};
