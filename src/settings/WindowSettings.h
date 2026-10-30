// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

// How the main window behaves with the tray, persisted in QSettings.
class WindowSettings : public QObject
{
    Q_OBJECT
    // Close button hides the window to the tray instead of quitting (default on).
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY changed)
    // Minimize hides the window to the tray instead of the taskbar (default off).
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray NOTIFY changed)

public:
    explicit WindowSettings(QObject *parent = nullptr);

    bool closeToTray() const { return m_closeToTray; }
    void setCloseToTray(bool enabled);

    bool minimizeToTray() const { return m_minimizeToTray; }
    void setMinimizeToTray(bool enabled);

signals:
    void changed();

private:
    bool m_closeToTray = true;
    bool m_minimizeToTray = false;
};
