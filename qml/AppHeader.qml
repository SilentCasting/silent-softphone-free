// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Rectangle {
    id: header

    readonly property bool canSwitchAccounts: Accounts.count > 1 && !Phone.inCall

    implicitHeight: 60
    color: Theme.surface

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 8

        // Active account; opens the account switcher when there is more than one.
        AbstractButton {
            id: accountButton

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 6
            Layout.bottomMargin: 6
            enabled: header.canSwitchAccounts
            hoverEnabled: true
            onClicked: switcher.open()

            background: Rectangle {
                radius: 10
                color: !accountButton.enabled ? "transparent"
                     : accountButton.down ? Theme.border
                     : accountButton.hovered ? Theme.surfaceAlt : "transparent"
            }

            contentItem: RowLayout {
                spacing: 12

                Avatar {
                    Layout.leftMargin: 8
                    text: Accounts.displayName
                    size: 36
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            Layout.fillWidth: true
                            text: Accounts.displayName || qsTr("No account")
                            color: Theme.text
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Icon {
                            visible: Accounts.count > 1
                            name: "expand_more"
                            size: 18
                            color: Theme.textMuted
                        }
                    }

                    StatusBadge {
                        Layout.fillWidth: true
                        status: Phone.registrationStatus
                        text: Phone.registrationText
                    }
                }
            }
        }

        IconButton {
            iconName: Theme.dark ? "light_mode" : "dark_mode"
            tooltip: Theme.dark ? qsTr("Light theme") : qsTr("Dark theme")
            onClicked: Theme.toggle()
        }
    }

    Popup {
        id: switcher

        x: 8
        y: header.height - 4
        width: Math.min(320, header.width - 16)
        padding: 4

        background: Rectangle {
            radius: 10
            color: Theme.surface
            border.color: Theme.border
        }

        contentItem: ListView {
            implicitHeight: contentHeight
            clip: true
            model: Accounts

            delegate: ItemDelegate {
                id: row

                required property int index
                required property string label
                readonly property bool active: index === Accounts.currentIndex

                width: ListView.view.width
                hoverEnabled: true
                onClicked: {
                    Accounts.activate(index)
                    switcher.close()
                }

                background: Rectangle {
                    radius: 6
                    color: row.hovered || row.down
                           ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16)
                           : "transparent"
                }

                contentItem: RowLayout {
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        text: row.label
                        color: Theme.text
                        font.weight: row.active ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                    }

                    Icon {
                        visible: row.active
                        name: "check"
                        size: 18
                        color: Theme.accent
                    }
                }
            }
        }
    }
}
