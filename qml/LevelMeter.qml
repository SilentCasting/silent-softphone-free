// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import SilentSoftphone.Core

// Live audio level bar with an icon, e.g. microphone or speaker.
RowLayout {
    id: root

    property string iconName
    property real level: 0      // 0..1
    property bool active: true  // false while muted / on hold

    spacing: 8

    Icon {
        name: root.iconName
        size: 16
        color: root.active ? Theme.textMuted : Theme.border
    }

    Rectangle {
        id: track

        Layout.fillWidth: true
        implicitHeight: 6
        radius: 3
        color: Theme.surfaceAlt

        Rectangle {
            // sqrt lifts quiet speech into view without pinning loud peaks at 100%.
            width: track.width * (root.active ? Math.min(1, Math.sqrt(root.level) * 1.15) : 0)
            height: parent.height
            radius: parent.radius
            color: Theme.success

            Behavior on width {
                NumberAnimation { duration: 90 }
            }
        }
    }
}
