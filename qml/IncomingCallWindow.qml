// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

// Always-on-top toast in the bottom-right corner while a call is ringing,
// visible even when the main window is minimized or behind other apps.
Window {
    id: popup

    objectName: "incomingCallWindow"
    readonly property var call: Phone.currentCall
    readonly property bool alerting: call !== null && call.state === CallSession.Alerting
    readonly property int margin: 24

    // No transient parent: an owned window would be hidden when the main window is minimized.
    transientParent: null
    // Qt.Tool windows hide when the app is inactive on macOS, so use a plain window there.
    flags: (Qt.platform.os === "osx" ? Qt.Window : Qt.Tool)
           | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"
    width: 360
    height: 116
    x: Screen.virtualX + Screen.width - width - margin
    y: Screen.virtualY + Screen.desktopAvailableHeight - height - margin
    visible: alerting
    title: qsTr("Incoming call")

    Rectangle {
        anchors.fill: parent
        radius: 16
        color: Theme.surface
        border.color: Theme.border

        // Accent strip so the toast stands out on any background.
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 1
            width: 4
            radius: 2
            color: Theme.accent
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 16
            spacing: 14

            Avatar {
                size: 48
                text: popup.call?.displayName ?? ""
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: qsTr("Incoming call")
                    color: Theme.accent
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                }

                Label {
                    Layout.fillWidth: true
                    text: popup.call?.displayName ?? ""
                    color: Theme.text
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: (popup.call?.remoteName ?? "") !== ""
                    text: popup.call?.number ?? ""
                    color: Theme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }

            IconButton {
                size: 44
                filled: true
                backgroundColor: Theme.danger
                iconColor: "white"
                iconName: "call_end"
                tooltip: qsTr("Decline")
                onClicked: Phone.hangup()
            }

            IconButton {
                size: 44
                filled: true
                backgroundColor: Theme.success
                iconColor: "white"
                iconName: "call"
                tooltip: qsTr("Answer")
                onClicked: {
                    Phone.answer()
                    Tray.showWindow() // also restores from the tray or a minimized state
                }
            }
        }
    }
}
