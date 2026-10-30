// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings/CallSettings.h"

#include <QSettings>

namespace {

constexpr auto kDtmfMethodKey = "calls/dtmfMethod";
constexpr auto kKeyTonesKey = "calls/keyTones";

DtmfMethod toDtmfMethod(int index)
{
    switch (index) {
    case int(DtmfMethod::Rfc2833): return DtmfMethod::Rfc2833;
    case int(DtmfMethod::SipInfo): return DtmfMethod::SipInfo;
    default:                       return DtmfMethod::Auto;
    }
}

} // namespace

CallSettings::CallSettings(QObject *parent)
    : QObject(parent)
{
    const QSettings settings;
    m_dtmfMethod = toDtmfMethod(settings.value(kDtmfMethodKey, int(DtmfMethod::Auto)).toInt());
    m_keyTones = settings.value(kKeyTonesKey, true).toBool();
}

void CallSettings::setDtmfMethodIndex(int index)
{
    const DtmfMethod method = toDtmfMethod(index);
    if (m_dtmfMethod == method)
        return;
    m_dtmfMethod = method;
    QSettings().setValue(kDtmfMethodKey, int(method));
    emit changed();
}

void CallSettings::setKeyTones(bool enabled)
{
    if (m_keyTones == enabled)
        return;
    m_keyTones = enabled;
    QSettings().setValue(kKeyTonesKey, enabled);
    emit changed();
}
