/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
import QtQuick
import QtQuick.Templates as T
import QtQml.Models
import QtQuick.Layouts


import VLC.Player
import VLC.Style
import VLC.Widgets as Widgets
import VLC.Util

GridView {
    id: root

    clip: true

    T.ScrollBar.vertical: Widgets.ScrollBarExt { }
    model: PlayerControlbarControls.controlList.length

    currentIndex: -1
    highlightFollowsCurrentItem: false

    cellWidth: 110
    cellHeight: 82

    boundsBehavior: Flickable.StopAtBounds

    property alias removeInfoRectVisible: removeInfoRect.visible

    signal controlDragStarted(int id)
    signal controlDragStopped(int id)


    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.View
    }

    MouseArea {
        anchors.fill: parent
        z: -1

        preventStealing: true
    }

    property Component implicitFlickableScrollHandler: DefaultFlickableScrollHandler { }

    // NOTE: This property can be set to null to prevent using a scroll handler:
    property FlickableScrollHandler scrollHandler: {
        if (interactive) {
            // JS ownership:
            return implicitFlickableScrollHandler.createObject(null, { target: root })
        } else {
            return null
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent

        z: 3

        function isFromList() {
            if (drag.source.objectName === "buttonsList")
                return true
            else
                return false
        }

        onDropped: (drop) => {
            if (isFromList())
                return

            drop.source.dndView.model.remove(drop.source.DelegateModel.itemsIndex)
            drop.accept(Qt.MoveAction)
        }
    }

    Rectangle {
        id: removeInfoRect
        anchors.fill: parent
        z: 2

        visible: false

        opacity: 0.8
        color: theme.bg.primary

        border.color: theme.border
        border.width: VLCStyle.dp(2, VLCStyle.scale)
        border.pixelAligned: (radius < Number.EPSILON)

        Text {
            anchors.centerIn: parent

            text: VLCIcons.del
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            font.pointSize: VLCStyle.fontHeight_xxxlarge

            font.family: VLCIcons.fontFamily
            color: theme.fg.secondary
        }

        MouseArea {
            anchors.fill: parent

            cursorShape: visible ? Qt.DragMoveCursor : Qt.ArrowCursor
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 1

        visible: buttonDragItem.Drag.active

        cursorShape: visible ? Qt.DragMoveCursor : Qt.ArrowCursor
    }

    delegate: MouseArea {
        width: cellWidth
        height: cellHeight

        hoverEnabled: true
        cursorShape: Qt.OpenHandCursor

        objectName: "buttonsList"

        drag.target: buttonDragItem

        drag.smoothed: false

        readonly property int mIndex: PlayerControlbarControls.controlList[model.index].id

        drag.onActiveChanged: {
            if (drag.active) {
                root.controlDragStarted(mIndex)

                buttonDragItem.text = PlayerControlbarControls.controlList[model.index].label
                buttonDragItem.Drag.source = this
                buttonDragItem.Drag.start()

                GridView.delayRemove = true
            } else {
                buttonDragItem.Drag.drop()

                root.controlDragStopped(mIndex)

                GridView.delayRemove = false
            }
        }

        onPressed: (mouse) => {
            const pos = mapToItem(buttonDragItem.parent, mouseX, mouseY)
            buttonDragItem.y = pos.y + VLCStyle.dragDelta
            buttonDragItem.x = pos.x + VLCStyle.dragDelta
        }

        Column {
            anchors.centerIn: parent
            spacing: 4

            Item {
                width: 32
                height: 32
                anchors.horizontalCenter: parent.horizontalCenter

                Canvas {
                    id: tileBorder
                    anchors.fill: parent
                    antialiasing: true

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = "#6F6F6F"
                        ctx.lineWidth = 1
                        ctx.setLineDash([3, 3])
                        ctx.beginPath()
                        ctx.moveTo(2, 0.5)
                        ctx.lineTo(width - 2, 0.5)
                        ctx.quadraticCurveTo(width - 0.5, 0.5, width - 0.5, 2)
                        ctx.lineTo(width - 0.5, height - 2)
                        ctx.quadraticCurveTo(width - 0.5, height - 0.5, width - 2, height - 0.5)
                        ctx.lineTo(2, height - 0.5)
                        ctx.quadraticCurveTo(0.5, height - 0.5, 0.5, height - 2)
                        ctx.lineTo(0.5, 2)
                        ctx.quadraticCurveTo(0.5, 0.5, 2, 0.5)
                        ctx.stroke()
                    }
                }

                EditorDummyButton {
                    anchors.fill: parent
                    color: "#D0D0D0"
                    font.pixelSize: 18
                    text: PlayerControlbarControls.controlList[model.index].label
                }
            }

            Text {
                width: root.cellWidth
                height: 42
                text: PlayerControlbarControls.controlList[model.index].text
                color: "#D0D0D0"
                font.pixelSize: 13
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
            }
        }

        onContainsMouseChanged: tileBorder.requestPaint()
    }
}
