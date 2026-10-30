// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/ThemeManager.h"

#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

namespace {

constexpr auto kModeKey = "ui/themeMode";

QColor hex(QRgb rgb)
{
    return QColor::fromRgb(rgb);
}

} // namespace

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    const int stored = QSettings().value(kModeKey, int(System)).toInt();
    m_mode = (stored >= System && stored <= Dark) ? Mode(stored) : System;

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        if (m_mode == System)
            updatePalette();
    });
    updatePalette();
}

void ThemeManager::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    QSettings().setValue(kModeKey, int(mode));
    emit modeChanged();
    updatePalette();
}

void ThemeManager::toggle()
{
    setMode(m_dark ? Light : Dark);
}

bool ThemeManager::systemPrefersDark()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void ThemeManager::updatePalette()
{
    m_dark = m_mode == Dark || (m_mode == System && systemPrefersDark());
    m_palette = m_dark ? darkPalette() : lightPalette();
    emit paletteChanged();
}

const ThemeManager::Palette &ThemeManager::lightPalette()
{
    static const Palette palette{
        hex(0xF4F6F9), hex(0xFFFFFF), hex(0xEBEEF3), hex(0xD9DEE6),
        hex(0x1B2230), hex(0x6B7585),
        hex(0x2563EB), hex(0xFFFFFF),
        hex(0x16A34A), hex(0xDC2626), hex(0xD97706),
    };
    return palette;
}

const ThemeManager::Palette &ThemeManager::darkPalette()
{
    static const Palette palette{
        hex(0x0F1318), hex(0x181D24), hex(0x222935), hex(0x2E3643),
        hex(0xE6EAF0), hex(0x8D97A6),
        hex(0x4C8DFF), hex(0xFFFFFF),
        hex(0x22C55E), hex(0xEF4444), hex(0xF59E0B),
    };
    return palette;
}
