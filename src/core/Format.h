// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

// "m:ss"-style call duration: 05:07, or 1:02:09 for calls longer than an hour.
inline QString formatDuration(int totalSeconds)
{
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds / 60) % 60;
    const int seconds = totalSeconds % 60;
    const auto twoDigits = [](int value) { return QString::number(value).rightJustified(2, u'0'); };

    if (hours > 0)
        return QStringLiteral("%1:%2:%3").arg(hours).arg(twoDigits(minutes), twoDigits(seconds));
    return QStringLiteral("%1:%2").arg(twoDigits(minutes), twoDigits(seconds));
}
