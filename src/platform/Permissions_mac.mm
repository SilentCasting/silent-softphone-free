// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// macOS privacy permissions: microphone through AVFoundation, notifications through
// UserNotifications. The prompt texts come from Info.plist (NS*UsageDescription).

#include "platform/Permissions.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QPointer>
#include <QUrl>

#import <AVFoundation/AVFoundation.h>
#import <UserNotifications/UserNotifications.h>

#include <functional>

namespace {

const QString kGranted = QStringLiteral("granted");
const QString kDenied = QStringLiteral("denied");
const QString kUndetermined = QStringLiteral("undetermined");

QString microphoneStatus()
{
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio]) {
    case AVAuthorizationStatusAuthorized:
        return kGranted;
    case AVAuthorizationStatusDenied:
    case AVAuthorizationStatusRestricted:
        return kDenied;
    case AVAuthorizationStatusNotDetermined:
        break;
    }
    return kUndetermined;
}

QString notificationStatus(UNAuthorizationStatus status)
{
    switch (status) {
    case UNAuthorizationStatusAuthorized:
    case UNAuthorizationStatusProvisional:
        return kGranted;
    case UNAuthorizationStatusDenied:
        return kDenied;
    default:
        return kUndetermined;
    }
}

// Completion handlers run on system queues; Qt objects are touched on the GUI thread only.
void onGuiThread(QPointer<Permissions> target, std::function<void(Permissions *)> action)
{
    QMetaObject::invokeMethod(qApp, [target, action] {
        if (target)
            action(target.data());
    }, Qt::QueuedConnection);
}

} // namespace

Permissions::Permissions(QObject *parent)
    : QObject(parent)
{
    m_microphone = microphoneStatus();
    m_notifications = kUndetermined;
    refresh();
}

bool Permissions::isSupported() const
{
    return true;
}

void Permissions::refresh()
{
    setMicrophone(microphoneStatus());

    QPointer<Permissions> self(this);
    [[UNUserNotificationCenter currentNotificationCenter]
        getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
            const QString status = notificationStatus(settings.authorizationStatus);
            onGuiThread(self, [status](Permissions *permissions) { permissions->setNotifications(status); });
        }];
}

void Permissions::requestMicrophone()
{
    const QString status = microphoneStatus();
    if (status == kDenied) {
        // macOS asks only once; after a refusal the switch lives in System Settings.
        openSettings(QStringLiteral("microphone"));
        return;
    }
    if (status == kGranted) {
        setMicrophone(status);
        return;
    }

    QPointer<Permissions> self(this);
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio
                             completionHandler:^(BOOL) {
                                 onGuiThread(self, [](Permissions *permissions) { permissions->refresh(); });
                             }];
}

void Permissions::requestNotifications()
{
    if (m_notifications == kDenied) {
        openSettings(QStringLiteral("notifications"));
        return;
    }

    QPointer<Permissions> self(this);
    [[UNUserNotificationCenter currentNotificationCenter]
        requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                      completionHandler:^(BOOL, NSError *) {
                          onGuiThread(self, [](Permissions *permissions) { permissions->refresh(); });
                      }];
}

void Permissions::requestMissing()
{
    if (microphoneStatus() == kUndetermined)
        requestMicrophone();

    // The notification status arrives asynchronously, so ask the center directly.
    QPointer<Permissions> self(this);
    [[UNUserNotificationCenter currentNotificationCenter]
        getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
            if (settings.authorizationStatus != UNAuthorizationStatusNotDetermined)
                return;
            onGuiThread(self, [](Permissions *permissions) { permissions->requestNotifications(); });
        }];
}

void Permissions::openSettings(const QString &permission) const
{
    // Deep links into System Settings → Privacy & Security / Notifications.
    QString url;
    if (permission == QLatin1String("microphone"))
        url = QStringLiteral("x-apple.systempreferences:com.apple.preference.security?Privacy_Microphone");
    else if (permission == QLatin1String("localNetwork"))
        url = QStringLiteral("x-apple.systempreferences:com.apple.preference.security?Privacy_LocalNetwork");
    else if (permission == QLatin1String("notifications"))
        url = QStringLiteral("x-apple.systempreferences:com.apple.Notifications-Settings.extension");
    else
        url = QStringLiteral("x-apple.systempreferences:com.apple.preference.security");
    QDesktopServices::openUrl(QUrl(url));
}

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
