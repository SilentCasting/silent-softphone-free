// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/Diagnostics.h"

#include "core/MockSipService.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

Diagnostics::Diagnostics(MockSipService *mock, const QString &logFile, QObject *parent)
    : QObject(parent)
    , m_mock(mock)
    , m_logFile(logFile)
{
}

void Diagnostics::simulateIncomingCall()
{
    if (m_mock)
        m_mock->simulateIncomingCall();
}

void Diagnostics::openLogFolder() const
{
    if (!m_logFile.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_logFile).absolutePath()));
}
