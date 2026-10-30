// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

AbstractButton {
    id: control

    property string iconName
    property color iconColor: Theme.text
    property color backgroundColor: Theme.surfaceAlt
    property bool filled: false
    property real size: 40
    property string tooltip

    implicitWidth: size
    implicitHeight: size
    hoverEnabled: true
    focusPolicy: Qt.NoFocus
    opacity: enabled ? 1 : 0.4

    ToolTip.visible: hovered && tooltip.length > 0
    ToolTip.text: tooltip
    ToolTip.delay: 600

    background: Rectangle {
        radius: width / 2
        color: control.filled
            ? (control.down ? Qt.darker(control.backgroundColor, 1.15)
                            : control.hovered ? Qt.lighter(control.backgroundColor, 1.08)
                                              : control.backgroundColor)
            : (control.down ? Theme.border : control.hovered ? Theme.surfaceAlt : "transparent")
    }

    contentItem: Item {
        Icon {
            anchors.centerIn: parent
            name: control.iconName
            color: control.iconColor
            size: Math.round(control.size * 0.5)
        }
    }
}
