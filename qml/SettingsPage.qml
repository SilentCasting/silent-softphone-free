// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

Flickable {
    id: root

    // Account shown in the form; saving it also makes it the active one.
    property int editedAccount: -1

    function loadAccount(index) {
        editedAccount = index
        const account = Accounts.get(index)
        displayNameField.text = account.displayName ?? ""
        usernameField.text = account.username ?? ""
        domainField.text = account.domain ?? ""
        passwordField.text = account.password ?? ""
        transportBox.currentIndex = account.transport ?? 0
        accountBox.currentIndex = index
    }

    clip: true
    contentWidth: width
    contentHeight: column.implicitHeight + 32
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar {}

    Component.onCompleted: loadAccount(Accounts.currentIndex)

    ColumnLayout {
        id: column
        x: 16
        y: 16
        width: root.width - 32
        spacing: 20

        SettingsSection {
            title: qsTr("SIP accounts")

            RowLayout {
                Layout.fillWidth: true
                spacing: 4

                AppComboBox {
                    id: accountBox
                    objectName: "accountBox"
                    Layout.fillWidth: true
                    model: Accounts
                    textRole: "label"
                    enabled: Accounts.count > 0
                    onActivated: (index) => root.loadAccount(index)
                }

                IconButton {
                    iconName: "add"
                    tooltip: qsTr("Add account")
                    onClicked: root.loadAccount(Accounts.add())
                }

                IconButton {
                    iconName: "delete"
                    iconColor: Theme.textMuted
                    tooltip: qsTr("Delete account")
                    enabled: root.editedAccount >= 0 && !Phone.inCall
                    onClicked: {
                        Accounts.remove(root.editedAccount)
                        root.loadAccount(Accounts.currentIndex)
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: Accounts.count > 0
                text: root.editedAccount === Accounts.currentIndex
                      ? qsTr("Active account")
                      : qsTr("Not active — save to switch to this account")
                color: root.editedAccount === Accounts.currentIndex ? Theme.success : Theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                visible: Accounts.count === 0
                text: qsTr("No accounts yet. Press + to add your SIP account.")
                color: Theme.textMuted
                wrapMode: Text.WordWrap
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: Accounts.count > 0
                spacing: 10

                LabeledField {
                    id: displayNameField
                    label: qsTr("Display name")
                }

                LabeledField {
                    id: usernameField
                    label: qsTr("Username / extension")
                }

                LabeledField {
                    id: domainField
                    label: qsTr("Domain / server")
                    placeholderText: "pbx.example.com"
                }

                LabeledField {
                    id: passwordField
                    label: qsTr("Password")
                    echoMode: TextInput.Password
                }

                Label {
                    text: qsTr("Transport")
                    color: Theme.textMuted
                    font.pixelSize: 12
                }

                AppComboBox {
                    id: transportBox
                    objectName: "transportBox"
                    Layout.fillWidth: true
                    model: ["UDP", "TCP", "TLS"]
                }

                PrimaryButton {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    enabled: !Phone.inCall
                    text: root.editedAccount === Accounts.currentIndex ? qsTr("Save and register")
                                                                       : qsTr("Save and use this account")
                    onClicked: {
                        Accounts.update(root.editedAccount, displayNameField.text, usernameField.text,
                                        domainField.text, passwordField.text, transportBox.currentIndex)
                        Accounts.activate(root.editedAccount)
                    }
                }
            }
        }

        SettingsSection {
            title: qsTr("Calls")

            Label {
                text: qsTr("DTMF method")
                color: Theme.textMuted
                font.pixelSize: 12
            }

            AppComboBox {
                Layout.fillWidth: true
                // Order matches DtmfMethod: Auto, RFC 2833, SIP INFO.
                model: [qsTr("Auto (RFC 2833, then SIP INFO)"), "RFC 2833", "SIP INFO"]
                currentIndex: CallSettings.dtmfMethod
                onActivated: (index) => CallSettings.dtmfMethod = index
            }

            AppCheckBox {
                text: qsTr("Play key tones")
                checked: CallSettings.keyTones
                onToggled: CallSettings.keyTones = checked
            }
        }

        SettingsSection {
            title: qsTr("Audio codecs")

            Label {
                Layout.fillWidth: true
                text: qsTr("Higher in the list is preferred. Changes apply to new calls.")
                color: Theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            Repeater {
                id: codecList
                model: Codecs

                delegate: RowLayout {
                    id: codecRow

                    required property int index
                    required property string name
                    required property string rate
                    required property bool codecEnabled

                    Layout.fillWidth: true
                    spacing: 4

                    AppCheckBox {
                        checked: codecRow.codecEnabled
                        onToggled: Codecs.setEnabled(codecRow.index, checked)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: codecRow.name
                        color: codecRow.codecEnabled ? Theme.text : Theme.textMuted
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Label {
                        text: codecRow.rate
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }

                    IconButton {
                        size: 32
                        iconName: "arrow_upward"
                        iconColor: Theme.textMuted
                        enabled: codecRow.index > 0
                        tooltip: qsTr("Move up")
                        onClicked: Codecs.move(codecRow.index, codecRow.index - 1)
                    }

                    IconButton {
                        size: 32
                        iconName: "arrow_downward"
                        iconColor: Theme.textMuted
                        enabled: codecRow.index < codecList.count - 1
                        tooltip: qsTr("Move down")
                        onClicked: Codecs.move(codecRow.index, codecRow.index + 1)
                    }
                }
            }
        }

        SettingsSection {
            title: qsTr("Appearance")

            Label {
                text: qsTr("Theme")
                color: Theme.textMuted
                font.pixelSize: 12
            }

            AppComboBox {
                Layout.fillWidth: true
                // Order matches ThemeManager::Mode: System, Light, Dark.
                model: [qsTr("System"), qsTr("Light"), qsTr("Dark")]
                currentIndex: Theme.mode
                onActivated: (index) => Theme.mode = index
            }
        }

        SettingsSection {
            visible: Tray.available || AutoStart.supported
            title: qsTr("Window and startup")

            AppCheckBox {
                Layout.fillWidth: true
                visible: Tray.available
                text: qsTr("Closing the window keeps Silent Softphone Free running in the tray")
                checked: WindowSettings.closeToTray
                onToggled: WindowSettings.closeToTray = checked
            }

            AppCheckBox {
                Layout.fillWidth: true
                visible: Tray.available
                text: qsTr("Minimize to the tray")
                checked: WindowSettings.minimizeToTray
                onToggled: WindowSettings.minimizeToTray = checked
            }

            AppCheckBox {
                Layout.fillWidth: true
                visible: AutoStart.supported
                text: qsTr("Start with the system (minimized to the tray)")
                checked: AutoStart.enabled
                onToggled: AutoStart.enabled = checked
            }
        }

        SettingsSection {
            title: qsTr("Diagnostics")

            Label {
                Layout.fillWidth: true
                visible: Diagnostics.logFile !== ""
                text: qsTr("SIP log: %1").arg(Diagnostics.logFile)
                color: Theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WrapAnywhere
            }

            Button {
                Layout.fillWidth: true
                visible: Diagnostics.logFile !== ""
                text: qsTr("Open log folder")
                onClicked: Diagnostics.openLogFolder()
            }

            Label {
                Layout.fillWidth: true
                visible: Diagnostics.mockBackend
                text: qsTr("The SIP backend is a mock. Use it to try call flows without a PBX.")
                color: Theme.textMuted
                wrapMode: Text.WordWrap
            }

            Button {
                Layout.fillWidth: true
                visible: Diagnostics.mockBackend
                text: qsTr("Simulate incoming call")
                enabled: !Phone.inCall
                onClicked: Diagnostics.simulateIncomingCall()
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Silent Softphone Free %1").arg(Qt.application.version)
            color: Theme.textMuted
            font.pixelSize: 12
        }
    }
}
