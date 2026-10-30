// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/Logging.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>

namespace {

QFile *g_file = nullptr;
QMutex g_mutex; // PJSIP logs from its media threads too
QtMessageHandler g_previousHandler = nullptr;

const char *levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:    return "debug";
    case QtInfoMsg:     return "info";
    case QtWarningMsg:  return "warning";
    case QtCriticalMsg: return "critical";
    case QtFatalMsg:    return "fatal";
    }
    return "";
}

void fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    {
        const QMutexLocker lock(&g_mutex);
        if (g_file) {
            const QString line = QStringLiteral("%1 %2 %3: %4\n")
                                     .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")),
                                          QLatin1String(levelName(type)),
                                          QLatin1String(context.category ? context.category : "default"),
                                          message);
            g_file->write(line.toUtf8());
            g_file->flush();
        }
    }
    if (g_previousHandler)
        g_previousHandler(type, context, message);
}

} // namespace

QString Logging::installFileLog()
{
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                            + QStringLiteral("/logs");
    if (!QDir().mkpath(directory))
        return {};

    const QString path = directory + QStringLiteral("/silentsoftphone.log");
    QFile::remove(path + QStringLiteral(".1"));
    QFile::rename(path, path + QStringLiteral(".1"));

    auto *file = new QFile(path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        delete file;
        return {};
    }
    g_file = file; // lives until process exit, like the handler
    g_previousHandler = qInstallMessageHandler(fileMessageHandler);
    return QDir::toNativeSeparators(path);
}
