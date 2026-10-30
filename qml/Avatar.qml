// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

Rectangle {
    id: root

    property string text
    property string presence: ""   // "" hides the presence dot
    property real size: 40

    // Names get initials; bare numbers like "+380..." get a person icon instead.
    readonly property bool hasName: {
        const first = text.trim().charAt(0)
        return first.toUpperCase() !== first.toLowerCase()
    }

    function initials(value) {
        const parts = value.trim().split(/\s+/).filter(part => part.length > 0)
        if (parts.length === 0)
            return "?"
        return parts.slice(0, 2).map(part => part[0].toUpperCase()).join("")
    }

    function presenceColor(value) {
        switch (value) {
        case "online": return Theme.success
        case "busy":   return Theme.danger
        case "away":   return Theme.warning
        default:       return Theme.textMuted
        }
    }

    implicitWidth: size
    implicitHeight: size
    radius: size / 2
    color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16)

    Icon {
        anchors.centerIn: parent
        visible: !root.hasName
        name: "person"
        size: Math.round(root.size * 0.55)
        color: Theme.accent
    }

    Label {
        anchors.centerIn: parent
        visible: root.hasName
        text: root.initials(root.text)
        color: Theme.accent
        font.pixelSize: Math.round(root.size * 0.38)
        font.weight: Font.DemiBold
    }

    Rectangle {
        visible: root.presence.length > 0
        width: Math.round(root.size * 0.3)
        height: width
        radius: width / 2
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: root.presenceColor(root.presence)
        border.width: 2
        border.color: Theme.surface
    }
}
