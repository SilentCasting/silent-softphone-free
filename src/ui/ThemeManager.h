// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QObject>

// Light/dark palettes for QML. In System mode it follows the OS color scheme live.
class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool dark READ isDark NOTIFY paletteChanged)
    Q_PROPERTY(QColor background READ background NOTIFY paletteChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY paletteChanged)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY paletteChanged)
    Q_PROPERTY(QColor border READ border NOTIFY paletteChanged)
    Q_PROPERTY(QColor text READ text NOTIFY paletteChanged)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY paletteChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY paletteChanged)
    Q_PROPERTY(QColor onAccent READ onAccent NOTIFY paletteChanged)
    Q_PROPERTY(QColor success READ success NOTIFY paletteChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY paletteChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY paletteChanged)

public:
    enum Mode { System, Light, Dark };
    Q_ENUM(Mode)

    explicit ThemeManager(QObject *parent = nullptr);

    Mode mode() const { return m_mode; }
    void setMode(Mode mode);
    bool isDark() const { return m_dark; }

    QColor background() const { return m_palette.background; }
    QColor surface() const { return m_palette.surface; }
    QColor surfaceAlt() const { return m_palette.surfaceAlt; }
    QColor border() const { return m_palette.border; }
    QColor text() const { return m_palette.text; }
    QColor textMuted() const { return m_palette.textMuted; }
    QColor accent() const { return m_palette.accent; }
    QColor onAccent() const { return m_palette.onAccent; }
    QColor success() const { return m_palette.success; }
    QColor danger() const { return m_palette.danger; }
    QColor warning() const { return m_palette.warning; }

    // Flips between explicit Light and Dark (leaves System mode).
    Q_INVOKABLE void toggle();

signals:
    void modeChanged();
    void paletteChanged();

private:
    struct Palette
    {
        QColor background, surface, surfaceAlt, border;
        QColor text, textMuted;
        QColor accent, onAccent;
        QColor success, danger, warning;
    };

    static const Palette &lightPalette();
    static const Palette &darkPalette();
    static bool systemPrefersDark();
    void updatePalette();

    Mode m_mode = System;
    bool m_dark = false;
    Palette m_palette;
};
