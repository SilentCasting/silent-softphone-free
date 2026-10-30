// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Item {
    id: root

    readonly property var call: Phone.currentCall
    readonly property bool inCall: call !== null
    readonly property bool alerting: inCall && call.state === CallSession.Alerting
    readonly property bool connected: inCall && (call.state === CallSession.Active || call.state === CallSession.Held)

    function isDialKey(text) {
        return text.length === 1 && "0123456789*#".indexOf(text) >= 0
    }

    function pressKey(key) {
        if (inCall) {
            Phone.sendDtmf(key)
        } else {
            numberField.insert(numberField.cursorPosition, key)
            numberField.forceActiveFocus()
            Phone.playKeyTone(key)
        }
    }

    function dialEntered() {
        const number = numberField.text.trim()
        if (number.length === 0)
            return
        Phone.dial(number)
        numberField.clear()
    }

    // During a call the number field is hidden; take focus so typed digits become DTMF.
    onInCallChanged: if (inCall) root.forceActiveFocus()
    Keys.onPressed: (event) => {
        if (root.inCall && root.isDialKey(event.text)) {
            root.pressKey(event.text)
            event.accepted = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        CallPanel {
            visible: root.inCall
            call: root.call
            Layout.fillWidth: true
        }

        RowLayout {
            visible: !root.inCall
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: numberField
                Layout.fillWidth: true
                placeholderText: qsTr("Number or SIP URI")
                font.pixelSize: 20
                horizontalAlignment: TextInput.AlignHCenter
                onAccepted: root.dialEntered()
                Keys.onPressed: (event) => {
                    if (root.isDialKey(event.text))
                        Phone.playKeyTone(event.text)
                    event.accepted = false // still type the character
                }
            }

            IconButton {
                iconName: "backspace"
                iconColor: Theme.textMuted
                enabled: numberField.text.length > 0
                tooltip: qsTr("Delete (hold to clear)")
                onClicked: {
                    const pos = numberField.cursorPosition
                    if (pos > 0)
                        numberField.remove(pos - 1, pos)
                }
                onPressAndHold: numberField.clear()
            }
        }

        DialPad {
            visible: !root.alerting
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 320
            Layout.alignment: Qt.AlignHCenter
            onKeyPressed: (key) => root.pressKey(key)
        }

        Item {
            visible: root.alerting
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 28

            CallActionButton {
                visible: root.connected
                iconName: (root.call?.muted ?? false) ? "mic_off" : "mic"
                label: (root.call?.muted ?? false) ? qsTr("Unmute") : qsTr("Mute")
                checked: root.call?.muted ?? false
                onClicked: Phone.toggleMute()
            }

            IconButton {
                visible: !root.inCall || root.alerting
                enabled: root.alerting || numberField.text.trim().length > 0
                size: 60
                filled: true
                backgroundColor: Theme.success
                iconColor: "white"
                iconName: "call"
                tooltip: root.alerting ? qsTr("Answer") : qsTr("Call")
                onClicked: root.alerting ? Phone.answer() : root.dialEntered()
            }

            IconButton {
                visible: root.inCall
                size: 60
                filled: true
                backgroundColor: Theme.danger
                iconColor: "white"
                iconName: "call_end"
                tooltip: root.alerting ? qsTr("Decline") : qsTr("Hang up")
                onClicked: Phone.hangup()
            }

            CallActionButton {
                visible: root.connected
                iconName: (root.call?.onHold ?? false) ? "play_arrow" : "pause"
                label: (root.call?.onHold ?? false) ? qsTr("Resume") : qsTr("Hold")
                checked: root.call?.onHold ?? false
                onClicked: Phone.toggleHold()
            }
        }
    }
}
