// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/AutoStart.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

namespace {

#if defined(Q_OS_WIN)
QString runKey()
{
    return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

constexpr auto kRunValue = "Silent Softphone Free";
#elif defined(Q_OS_MACOS)
QString launchAgentPath()
{
    return QDir::homePath() + QStringLiteral("/Library/LaunchAgents/io.github.silentcasting.silentsoftphonefree.plist");
}
#endif

} // namespace

AutoStart::AutoStart(QObject *parent)
    : QObject(parent)
{
    // Keep the entry pointing at this executable, e.g. after the app was moved or updated.
    if (isEnabled())
        write();
}

bool AutoStart::isSupported() const
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return true;
#else
    return false;
#endif
}

bool AutoStart::isEnabled() const
{
#if defined(Q_OS_WIN)
    return QSettings(runKey(), QSettings::NativeFormat).contains(QLatin1String(kRunValue));
#elif defined(Q_OS_MACOS)
    return QFile::exists(launchAgentPath());
#else
    return false;
#endif
}

void AutoStart::setEnabled(bool enabled)
{
    if (!isSupported() || enabled == isEnabled())
        return;
    if (enabled)
        write();
    else
        remove();
    emit enabledChanged();
}

void AutoStart::write() const
{
    const QString executable = QCoreApplication::applicationFilePath();
#if defined(Q_OS_WIN)
    QSettings(runKey(), QSettings::NativeFormat)
        .setValue(QLatin1String(kRunValue),
                  QStringLiteral("\"%1\" --minimized").arg(QDir::toNativeSeparators(executable)));
#elif defined(Q_OS_MACOS)
    const QString path = launchAgentPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    file.write(QStringLiteral(R"plist(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>io.github.silentcasting.silentsoftphonefree</string>
    <key>ProgramArguments</key>
    <array>
        <string>%1</string>
        <string>--minimized</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
</dict>
</plist>
)plist").arg(executable.toHtmlEscaped()).toUtf8());
#else
    Q_UNUSED(executable)
#endif
}

void AutoStart::remove() const
{
#if defined(Q_OS_WIN)
    QSettings(runKey(), QSettings::NativeFormat).remove(QLatin1String(kRunValue));
#elif defined(Q_OS_MACOS)
    QFile::remove(launchAgentPath());
#endif
}
