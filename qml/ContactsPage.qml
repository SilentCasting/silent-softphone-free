// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search contacts")
                onTextChanged: Contacts.filterText = text
            }

            IconButton {
                iconName: "add"
                tooltip: qsTr("New contact")
                onClicked: addDialog.open()
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: Contacts
            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                id: row

                required property int index
                required property string name
                required property string number
                required property string presence

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

                    Avatar {
                        text: row.name
                        presence: row.presence
                        size: 40
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            text: row.name
                            color: Theme.text
                            font.pixelSize: 14
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: row.number
                            color: Theme.textMuted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    IconButton {
                        iconName: "call"
                        iconColor: Theme.success
                        tooltip: qsTr("Call")
                        onClicked: Phone.dial(row.number)
                    }

                    IconButton {
                        iconName: "delete"
                        iconColor: Theme.textMuted
                        tooltip: qsTr("Delete")
                        onClicked: Contacts.removeAt(row.index)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No contacts")
                color: Theme.textMuted
            }
        }
    }

    Dialog {
        id: addDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(340, parent.width - 32)
        modal: true
        title: qsTr("New contact")
        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            nameField.clear()
            numberField.clear()
            nameField.forceActiveFocus()
        }
        onAccepted: Contacts.addContact(nameField.text, numberField.text)

        ColumnLayout {
            width: parent.width
            spacing: 10

            TextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: qsTr("Name")
            }

            TextField {
                id: numberField
                Layout.fillWidth: true
                placeholderText: qsTr("Number or SIP URI")
                onAccepted: addDialog.accept()
            }
        }
    }
}
