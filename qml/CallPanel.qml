// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

// Compact card for the current call: who, state/timer, live audio levels, codec.
// Mute / hold / hang-up live in the dialer's bottom row so the keypad keeps its space.
Rectangle {
    id: root

    property var call: null
    readonly property int callState: call?.state ?? CallSession.Ended
    readonly property bool connected: callState === CallSession.Active || callState === CallSession.Held
    readonly property bool muted: call?.muted ?? false

    implicitHeight: content.implicitHeight + 32
    radius: 16
    color: Theme.surface
    border.color: Theme.border

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Avatar {
                size: 48
                text: root.call?.displayName ?? ""
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    text: root.call?.displayName ?? ""
                    color: Theme.text
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: (root.call?.remoteName ?? "") !== ""
                    text: root.call?.number ?? ""
                    color: Theme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.callState === CallSession.Active ? (root.call?.durationText ?? "")
                                                                : (root.call?.stateText ?? "")
                    color: root.callState === CallSession.Alerting ? Theme.accent
                         : root.callState === CallSession.Held ? Theme.warning
                         : Theme.textMuted
                    font.pixelSize: 13
                    font.features: { "tnum": 1 }
                }
            }
        }

        ColumnLayout {
            visible: root.connected
            Layout.fillWidth: true
            spacing: 6

            LevelMeter {
                Layout.fillWidth: true
                iconName: root.muted ? "mic_off" : "mic"
                level: root.call?.sendLevel ?? 0
                active: !root.muted && root.callState === CallSession.Active
            }

            LevelMeter {
                Layout.fillWidth: true
                iconName: "volume_up"
                level: root.call?.receiveLevel ?? 0
                active: root.callState === CallSession.Active
            }

            Label {
                visible: text.length > 0
                text: (root.call?.codec ?? "") !== "" ? qsTr("Codec: %1").arg(root.call.codec) : ""
                color: Theme.textMuted
                font.pixelSize: 11
            }
        }
    }
}
