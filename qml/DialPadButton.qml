// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

AbstractButton {
    id: control

    property string digit
    property string letters

    implicitWidth: 72
    implicitHeight: 56
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    background: Rectangle {
        radius: 12
        color: control.down ? Theme.border : control.hovered ? Theme.surfaceAlt : Theme.surface
        border.color: Theme.border
        border.width: 1
    }

    contentItem: Item {
        Column {
            anchors.centerIn: parent
            spacing: -2

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: control.digit
                color: Theme.text
                font.pixelSize: 22
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: control.letters.length > 0
                text: control.letters
                color: Theme.textMuted
                font.pixelSize: 9
                font.letterSpacing: 1
            }
        }
    }
}
