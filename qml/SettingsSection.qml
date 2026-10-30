// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

ColumnLayout {
    id: root

    property string title
    default property alias content: body.data

    Layout.fillWidth: true
    spacing: 8

    Label {
        text: root.title
        color: Theme.textMuted
        font.pixelSize: 11
        font.weight: Font.DemiBold
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 0.8
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: body.implicitHeight + 32
        radius: 12
        color: Theme.surface
        border.color: Theme.border

        ColumnLayout {
            id: body
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 16
            spacing: 10
        }
    }
}
