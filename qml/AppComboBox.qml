// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import SilentSoftphone.Core

// ComboBox with theme-aware popup rows. The Basic style paints the highlighted row
// with palette.light and palette.highlightedText, which is white on white in the
// light theme, so every ComboBox in the app uses this instead.
ComboBox {
    id: control

    // Elided label instead of the Basic style's text field, which clips long text.
    contentItem: Label {
        leftPadding: 6
        text: control.displayText
        color: Theme.text
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    delegate: ItemDelegate {
        id: row

        required property int index

        width: ListView.view ? ListView.view.width : control.width
        highlighted: control.highlightedIndex === index
        hoverEnabled: true

        contentItem: Label {
            text: control.textAt(row.index)
            color: Theme.text
            font.weight: control.currentIndex === row.index ? Font.DemiBold : Font.Normal
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            radius: 6
            color: row.highlighted || row.down
                   ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16)
                   : "transparent"
        }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            radius: 8
            color: Theme.surface
            border.color: Theme.border
        }
    }
}
