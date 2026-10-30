// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

ColumnLayout {
    property alias label: caption.text
    property alias text: field.text
    property alias placeholderText: field.placeholderText
    property alias echoMode: field.echoMode

    Layout.fillWidth: true
    spacing: 4

    Label {
        id: caption
        color: Theme.textMuted
        font.pixelSize: 12
    }

    TextField {
        id: field
        Layout.fillWidth: true
    }
}
