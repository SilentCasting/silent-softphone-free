// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

ColumnLayout {
    id: root

    property string iconName
    property string label
    property bool checked: false
    signal clicked()

    spacing: 4

    IconButton {
        Layout.alignment: Qt.AlignHCenter
        size: 48
        filled: true
        backgroundColor: root.checked ? Theme.accent : Theme.surfaceAlt
        iconColor: root.checked ? Theme.onAccent : Theme.text
        iconName: root.iconName
        onClicked: root.clicked()
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        text: root.label
        color: Theme.textMuted
        font.pixelSize: 11
    }
}
