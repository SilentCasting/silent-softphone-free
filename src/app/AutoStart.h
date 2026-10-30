// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

// "Start with the system": launches the app with --minimized at login.
// Windows: HKCU\Software\Microsoft\Windows\CurrentVersion\Run.
// macOS: ~/Library/LaunchAgents/io.github.silentcasting.silentsoftphonefree.plist.
class AutoStart : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool supported READ isSupported CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit AutoStart(QObject *parent = nullptr);

    bool isSupported() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);

signals:
    void enabledChanged();

private:
    void write() const;
    void remove() const;
};
