// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Rectangle {
    id: root

    property var tabs: []
    property int currentIndex: 0
    property bool topBorder: true
    signal selected(int index)

    implicitHeight: 62
    color: Theme.surface

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        y: root.topBorder ? 0 : parent.height - height
        height: 1
        color: Theme.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4

        Repeater {
            model: root.tabs

            delegate: AbstractButton {
                id: tab

                required property var modelData
                readonly property bool current: modelData.index === root.currentIndex
                readonly property color tint: current ? Theme.accent : Theme.textMuted

                Layout.fillWidth: true
                Layout.fillHeight: true
                hoverEnabled: true
                onClicked: root.selected(modelData.index)

                background: Rectangle {
                    radius: 10
                    color: tab.current ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.14)
                         : tab.hovered ? Theme.surfaceAlt : "transparent"
                }

                contentItem: Item {
                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Icon {
                            anchors.horizontalCenter: parent.horizontalCenter
                            name: tab.modelData.icon
                            size: 22
                            color: tab.tint
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: tab.modelData.label
                            font.pixelSize: 11
                            font.weight: tab.current ? Font.DemiBold : Font.Normal
                            color: tab.tint
                        }
                    }
                }
            }
        }
    }
}
