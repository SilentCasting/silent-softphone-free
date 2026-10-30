// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Item {
    id: root

    function kindIcon(kind) {
        return kind === "missed" ? "call_missed" : kind === "incoming" ? "call_received" : "call_made"
    }

    function kindColor(kind) {
        return kind === "missed" ? Theme.danger : kind === "incoming" ? Theme.success : Theme.accent
    }

    function kindLabel(kind) {
        return kind === "missed" ? qsTr("Missed") : kind === "incoming" ? qsTr("Incoming") : qsTr("Outgoing")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: qsTr("Recent calls")
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.DemiBold
            }

            Button {
                text: qsTr("Clear")
                flat: true
                enabled: list.count > 0
                onClicked: History.clear()
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: History
            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                id: row

                required property string name
                required property string number
                required property string kind
                required property string timeText
                required property string durationText

                width: ListView.view.width
                height: 60
                hoverEnabled: true
                onDoubleClicked: Phone.dial(number)

                background: Rectangle {
                    radius: 10
                    color: row.down ? Theme.border : row.hovered ? Theme.surfaceAlt : "transparent"
                }

                contentItem: RowLayout {
                    spacing: 12

                    Icon {
                        name: root.kindIcon(row.kind)
                        color: root.kindColor(row.kind)
                        size: 20
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            text: row.name || row.number
                            color: row.kind === "missed" ? Theme.danger : Theme.text
                            font.pixelSize: 14
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: row.name ? row.number : root.kindLabel(row.kind)
                            color: Theme.textMuted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    ColumnLayout {
                        spacing: 2

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: row.timeText
                            color: Theme.textMuted
                            font.pixelSize: 12
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: row.durationText
                            color: Theme.textMuted
                            font.pixelSize: 12
                        }
                    }

                    IconButton {
                        iconName: "call"
                        iconColor: Theme.success
                        tooltip: qsTr("Call back")
                        onClicked: Phone.dial(row.number)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No calls yet")
                color: Theme.textMuted
            }
        }
    }
}
