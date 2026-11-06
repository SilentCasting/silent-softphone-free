// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

// One OS permission: what it is for, its status and a button to ask for it or to open
// System Settings (macOS asks only once; after a refusal only Settings can change it).
RowLayout {
    id: root

    property string title
    property string detail
    property string status         // "granted" | "denied" | "undetermined"
    property bool showStatus: true // false when the OS doesn't report the status
    signal requested()

    Layout.fillWidth: true
    spacing: 8

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 1

        Label {
            text: root.title
            color: Theme.text
            font.weight: Font.DemiBold
        }

        Label {
            Layout.fillWidth: true
            visible: text.length > 0
            text: root.detail
            color: Theme.textMuted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    Label {
        visible: root.showStatus
        text: root.status === "granted" ? qsTr("Allowed")
            : root.status === "denied" ? qsTr("Not allowed")
            : qsTr("Not asked yet")
        color: root.status === "granted" ? Theme.success
             : root.status === "denied" ? Theme.danger
             : Theme.warning
        font.pixelSize: 12
    }

    Button {
        visible: !root.showStatus || root.status !== "granted"
        text: root.showStatus && root.status === "undetermined" ? qsTr("Allow") : qsTr("Open Settings")
        onClicked: root.requested()
    }
}
