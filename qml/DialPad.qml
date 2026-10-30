// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

GridLayout {
    id: root

    signal keyPressed(string key)

    // Buttons shrink to 40 px when space is tight (in a call at minimum window height)
    // and stop growing at 60 px, so the keypad never stretches across a tall window.
    columns: 3
    rowSpacing: 8
    columnSpacing: 10
    Layout.minimumHeight: 4 * 40 + 3 * rowSpacing
    Layout.maximumHeight: 4 * 60 + 3 * rowSpacing

    Repeater {
        model: [
            { digit: "1", letters: "" },    { digit: "2", letters: "ABC" }, { digit: "3", letters: "DEF" },
            { digit: "4", letters: "GHI" }, { digit: "5", letters: "JKL" }, { digit: "6", letters: "MNO" },
            { digit: "7", letters: "PQRS" },{ digit: "8", letters: "TUV" }, { digit: "9", letters: "WXYZ" },
            { digit: "*", letters: "" },    { digit: "0", letters: "+" },   { digit: "#", letters: "" }
        ]

        delegate: DialPadButton {
            required property var modelData

            digit: modelData.digit
            letters: modelData.letters
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 40
            Layout.maximumHeight: 60

            onClicked: root.keyPressed(digit)
            // Long-press on 0 enters "+", as on mobile phones.
            onPressAndHold: root.keyPressed(digit === "0" ? "+" : digit)
        }
    }
}
