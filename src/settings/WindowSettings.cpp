// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings/WindowSettings.h"

#include <QSettings>

namespace {

constexpr auto kCloseToTrayKey = "window/closeToTray";
constexpr auto kMinimizeToTrayKey = "window/minimizeToTray";

} // namespace

WindowSettings::WindowSettings(QObject *parent)
    : QObject(parent)
{
    const QSettings settings;
    m_closeToTray = settings.value(kCloseToTrayKey, true).toBool();
    m_minimizeToTray = settings.value(kMinimizeToTrayKey, false).toBool();
}

void WindowSettings::setCloseToTray(bool enabled)
{
    if (m_closeToTray == enabled)
        return;
    m_closeToTray = enabled;
    QSettings().setValue(kCloseToTrayKey, enabled);
    emit changed();
}

void WindowSettings::setMinimizeToTray(bool enabled)
{
    if (m_minimizeToTray == enabled)
        return;
    m_minimizeToTray = enabled;
    QSettings().setValue(kMinimizeToTrayKey, enabled);
    emit changed();
}
