// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>

class MockSipService;

// Troubleshooting helpers for QML: the log file, and call simulation when the mock
// SIP backend is active.
class Diagnostics : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool mockBackend READ isMockBackend CONSTANT)
    Q_PROPERTY(QString logFile READ logFile CONSTANT)

public:
    Diagnostics(MockSipService *mock, const QString &logFile, QObject *parent = nullptr);

    bool isMockBackend() const { return m_mock != nullptr; }
    QString logFile() const { return m_logFile; }

    Q_INVOKABLE void simulateIncomingCall();
    Q_INVOKABLE void openLogFolder() const;

private:
    QPointer<MockSipService> m_mock;
    const QString m_logFile;
};
