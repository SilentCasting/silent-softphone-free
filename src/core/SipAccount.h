// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

// Plain value type handed to the SIP stack; persistence lives in AccountManager.
struct SipAccount
{
    enum class Transport { Udp, Tcp, Tls };

    QString displayName;
    QString username;
    QString domain;
    QString password;
    Transport transport = Transport::Udp;

    bool isValid() const { return !username.isEmpty() && !domain.isEmpty(); }
    QString uri() const { return QStringLiteral("sip:%1@%2").arg(username, domain); }
};

// "sip:1002@pbx.example.com" -> "1002"
inline QString sipUserPart(const QString &uri)
{
    QString user = uri;
    if (user.startsWith(QLatin1String("sip:")) || user.startsWith(QLatin1String("tel:")))
        user.remove(0, 4);
    else if (user.startsWith(QLatin1String("sips:")))
        user.remove(0, 5);
    const qsizetype at = user.indexOf(u'@');
    return at >= 0 ? user.left(at) : user;
}
