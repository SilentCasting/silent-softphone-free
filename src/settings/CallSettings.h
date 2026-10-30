// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/SipTypes.h"

#include <QObject>

// Call-related preferences shown in Settings → Calls, persisted in QSettings.
class CallSettings : public QObject
{
    Q_OBJECT
    // Index into DtmfMethod: 0 Auto, 1 RFC 2833, 2 SIP INFO.
    Q_PROPERTY(int dtmfMethod READ dtmfMethodIndex WRITE setDtmfMethodIndex NOTIFY changed)
    Q_PROPERTY(bool keyTones READ keyTones WRITE setKeyTones NOTIFY changed)

public:
    explicit CallSettings(QObject *parent = nullptr);

    DtmfMethod dtmfMethod() const { return m_dtmfMethod; }
    int dtmfMethodIndex() const { return int(m_dtmfMethod); }
    void setDtmfMethodIndex(int index);

    bool keyTones() const { return m_keyTones; }
    void setKeyTones(bool enabled);

signals:
    void changed();

private:
    DtmfMethod m_dtmfMethod = DtmfMethod::Auto;
    bool m_keyTones = true;
};
