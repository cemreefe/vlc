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
 * Foundation, Inc., 51 Franklin Street, Boston MA 02110-1301, USA.
 *****************************************************************************/

import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QtQml.Models
import QtQuick.Window


import VLC.MainInterface
import VLC.Style
import VLC.Playlist
import VLC.Widgets as Widgets
import VLC.Menus as Menus
import VLC.Player
import VLC.Util

T.ToolBar {
    id: root

    Accessible.name: qsTr("Menu")
    AccessibleCompat.id: "menuTopbar"

    // For now, used for d&d functionality
    // Not strictly necessary to set
    property PlaylistPane plListView: null

    property bool _showCSD: MainCtx.clientSideDecoration && !(MainCtx.intfMainWindow.visibility === Window.FullScreen)

    hoverEnabled: true

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    ColorContext {
        id: theme
        colorSet: ColorContext.Window
    }

    background: Widgets.AcrylicBackground {
        tintColor: theme.bg.secondary
        alternativeColor: theme.bg.primary
    }

    contentItem: Item {
        implicitHeight: VLCStyle.globalToolbar_height
            + VLCStyle.margin_xxxsmall

        //drag and dbl click the titlebar in CSD mode
        Loader {
            z:-1
            anchors.fill: parent
            active: root._showCSD
            sourceComponent: Widgets.CSDTitlebarTapNDrapHandler {
            }
        }

        Menus.QmlMenuBar {
            id: menubar

            ctx: MainCtx
            menubar: appMenuButton
        }

        Widgets.BannerCone {
            id: cone

            anchors {
                left: parent.left
                leftMargin: VLCStyle.applicationHorizontalMargin
                verticalCenter: parent.verticalCenter
            }

            height: VLCStyle.icon_normal
            width: VLCStyle.icon_normal

            color: theme.accent
        }

        Widgets.IconToolButton {
            id: appMenuButton

            anchors {
                left: cone.right
                leftMargin: VLCStyle.margin_xsmall
                verticalCenter: parent.verticalCenter
            }

            text: VLCIcons.ellipsis
            description: qsTr("Application menu")
            font.pixelSize: VLCStyle.icon_normal

            onClicked: menubar.popupAppMenu(appMenuButton)
        }

        T.Label {
            anchors.centerIn: parent

            text: (Player.title && Player.title.length > 0) ? Player.title : "VLC Media Player"
            color: theme.fg.secondary
            font.pixelSize: VLCStyle.fontSize_normal

            visible: !root._showCSD || (csdButtons.height < VLCStyle.applicationVerticalMargin)
        }

        Loader {
            id: csdButtons
            anchors.right: parent.right
            height: Math.min(parent.implicitHeight, VLCStyle.globalToolbar_height)
            active: root._showCSD
            source: VLCStyle.palette.hasCSDImage
                      ? "qrc:///qt/qml/VLC/Widgets/CSDThemeButtonSet.qml"
                      : "qrc:///qt/qml/VLC/Widgets/CSDWindowButtonSet.qml"
        }

        Keys.priority: Keys.AfterItem
        Keys.onPressed: (event) => root.Navigation.defaultKeyAction(event)
    }
}
