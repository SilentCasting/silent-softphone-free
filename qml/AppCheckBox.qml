// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

// CheckBox drawn in theme colors (the Basic style's indicator ignores the accent).
CheckBox {
    id: control

    hoverEnabled: true

    indicator: Rectangle {
        implicitWidth: 20
        implicitHeight: 20
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        radius: 5
        color: control.checked ? Theme.accent : "transparent"
        border.width: control.checked ? 0 : 1.5
        border.color: control.hovered ? Theme.accent : Theme.textMuted

        Icon {
            anchors.centerIn: parent
            visible: control.checked
            name: "check"
            size: 16
            color: Theme.onAccent
        }
    }

    contentItem: Label {
        leftPadding: control.indicator.width + (control.text.length > 0 ? control.spacing : 0)
        text: control.text
        color: Theme.text
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}
