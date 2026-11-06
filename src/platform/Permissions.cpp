// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Platforms without per-app privacy prompts for these features: everything is granted.
// The macOS implementation is Permissions_mac.mm.

#include "platform/Permissions.h"

Permissions::Permissions(QObject *parent)
    : QObject(parent)
{
}

bool Permissions::isSupported() const
{
    return false;
}

void Permissions::refresh() {}
void Permissions::requestMicrophone() {}
void Permissions::requestNotifications() {}
void Permissions::requestMissing() {}
void Permissions::openSettings(const QString &) const {}

void Permissions::setMicrophone(const QString &status)
{
    if (m_microphone == status)
        return;
    m_microphone = status;
    emit changed();
}

void Permissions::setNotifications(const QString &status)
{
    if (m_notifications == status)
        return;
    m_notifications = status;
    emit changed();
}
