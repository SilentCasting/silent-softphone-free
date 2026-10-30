// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

enum class DtmfMethod {
    Auto,    // RFC 2833, falling back to SIP INFO when the remote side doesn't support it
    Rfc2833,
    SipInfo
};

struct SipCodec
{
    QString id;       // stack identifier, e.g. "PCMA/8000/1"
    int priority = 0; // 0 = disabled, higher = preferred
};

// Live media state of a call; PhoneController polls it while the call is up.
struct CallMediaInfo
{
    double sendLevel = 0;    // 0..1, our audio going to the remote side
    double receiveLevel = 0; // 0..1, audio coming from the remote side
    QString codec;           // e.g. "PCMA 8 kHz"
};
