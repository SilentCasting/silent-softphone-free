// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

RowLayout {
    id: root

    property string status   // "registered" | "registering" | "failed" | "offline"
    property alias text: label.text

    spacing: 6

    Rectangle {
        implicitWidth: 8
        implicitHeight: 8
        radius: 4
        color: root.status === "registered" ? Theme.success
             : root.status === "registering" ? Theme.warning
             : root.status === "failed" ? Theme.danger
             : Theme.textMuted
    }

    Label {
        id: label
        Layout.fillWidth: true
        color: Theme.textMuted
        font.pixelSize: 12
        elide: Text.ElideRight
    }
}
