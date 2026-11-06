// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

// Operating-system privacy permissions the app depends on. On macOS calls need the
// microphone, and incoming-call and missed-call alerts need notifications; other
// platforms don't gate these, so there everything reports "granted".
//
// Status values: "granted", "denied" or "undetermined" (not asked yet).
class Permissions : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool supported READ isSupported CONSTANT)
    Q_PROPERTY(QString microphone READ microphone NOTIFY changed)
    Q_PROPERTY(QString notifications READ notifications NOTIFY changed)

public:
    explicit Permissions(QObject *parent = nullptr);

    bool isSupported() const;
    QString microphone() const { return m_microphone; }
    QString notifications() const { return m_notifications; }

    // Re-reads the statuses, e.g. after the user comes back from System Settings.
    Q_INVOKABLE void refresh();
    // Shows the system prompt if not asked yet; opens System Settings if denied.
    Q_INVOKABLE void requestMicrophone();
    Q_INVOKABLE void requestNotifications();
    // Asks for everything not asked yet, without opening System Settings. Used at startup.
    Q_INVOKABLE void requestMissing();
    // "microphone", "notifications" or "localNetwork".
    Q_INVOKABLE void openSettings(const QString &permission) const;

signals:
    void changed();

private:
    void setMicrophone(const QString &status);
    void setNotifications(const QString &status);

    QString m_microphone = QStringLiteral("granted");
    QString m_notifications = QStringLiteral("granted");
};
