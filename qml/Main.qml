// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SilentSoftphone.Core

ApplicationWindow {
    id: window

    // Below this width the app is one compact column with a bottom tab bar.
    // Above it, the dialer stays pinned on the left.
    readonly property bool wide: width >= 720
    readonly property var tabs: [
        { index: 0, label: qsTr("Dial"), icon: "dialpad" },
        { index: 1, label: qsTr("Contacts"), icon: "people" },
        { index: 2, label: qsTr("History"), icon: "history" },
        { index: 3, label: qsTr("Settings"), icon: "tune" }
    ]
    property int currentTab: 0
    // Set from C++ when launched with --minimized (autostart): start in the tray.
    property bool startHidden: false

    // minimumHeight fits a call (call card + keypad + controls) in the compact layout.
    width: 380
    height: 680
    minimumWidth: 320
    minimumHeight: 640
    visible: !startHidden
    title: qsTr("Silent Softphone Free")
    color: Theme.background

    // Basic-style controls (TextField, ComboBox, Dialog…) pick their colors from here.
    palette.window: Theme.background
    palette.windowText: Theme.text
    palette.base: Theme.background
    palette.alternateBase: Theme.surfaceAlt
    palette.text: Theme.text
    palette.button: Theme.surfaceAlt
    palette.buttonText: Theme.text
    palette.brightText: Theme.onAccent
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.onAccent
    palette.placeholderText: Theme.textMuted
    palette.light: Theme.surface
    palette.midlight: Theme.surfaceAlt
    palette.mid: Theme.border
    palette.dark: Theme.border
    palette.shadow: Theme.border
    palette.toolTipBase: Theme.surfaceAlt
    palette.toolTipText: Theme.text

    onWideChanged: if (wide && currentTab === 0) currentTab = 1

    Component.onCompleted: Tray.attach(window)

    // With a tray the window just hides on close and the app keeps taking calls,
    // unless the user turned that off. Without a tray, closing the last window quits.
    onClosing: {
        if (!Tray.available)
            return
        if (WindowSettings.closeToTray)
            Tray.notifyHiddenToTray()
        else
            Tray.quit()
    }

    onVisibilityChanged: (visibility) => {
        if (visibility === Window.Minimized && Tray.available && WindowSettings.minimizeToTray)
            Tray.hideWindow()
    }

    Connections {
        target: Tray
        function onSettingsRequested() {
            window.currentTab = 3
        }
    }

    Connections {
        target: Phone
        function onCurrentCallChanged() {
            if (Phone.inCall && !window.wide)
                window.currentTab = 0
        }
    }

    header: AppHeader {}

    IncomingCallWindow {}

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            DialerPage {
                visible: window.wide || window.currentTab === 0
                Layout.fillHeight: true
                Layout.fillWidth: !window.wide
                Layout.preferredWidth: window.wide ? 360 : -1
            }

            Rectangle {
                visible: window.wide
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: Theme.border
            }

            ColumnLayout {
                visible: window.wide || window.currentTab !== 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                NavBar {
                    visible: window.wide
                    Layout.fillWidth: true
                    tabs: window.tabs.slice(1)
                    currentIndex: window.currentTab
                    topBorder: false
                    onSelected: (index) => window.currentTab = index
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: Math.max(0, window.currentTab - 1)

                    ContactsPage {}
                    HistoryPage {}
                    SettingsPage {}
                }
            }
        }

        NavBar {
            visible: !window.wide
            Layout.fillWidth: true
            tabs: window.tabs
            currentIndex: window.currentTab
            onSelected: (index) => window.currentTab = index
        }
    }
}
