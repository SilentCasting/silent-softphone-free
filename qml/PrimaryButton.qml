// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

Button {
    id: control

    hoverEnabled: true

    contentItem: Label {
        text: control.text
        color: Theme.onAccent
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        implicitHeight: 40
        radius: 10
        opacity: control.enabled ? 1 : 0.5
        color: control.down ? Qt.darker(Theme.accent, 1.15)
             : control.hovered ? Qt.lighter(Theme.accent, 1.08)
             : Theme.accent
    }
}
