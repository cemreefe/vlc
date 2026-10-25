/*****************************************************************************
 * ToolsPanel.qml : right-side tools panel (Tool | Subtitles | Audio | Video)
 *****************************************************************************/
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts


import VLC.MainInterface
import VLC.Style
import VLC.Player
import VLC.Util
import VLC.Dialogs
import VLC.Widgets as Widgets

FocusScope {
    id: root

    signal requestClose()

    implicitWidth: VLCStyle.dp(300, VLCStyle.scale)

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.Window
    }

    QtObject {
        id: panelController

        signal requestBack()
        signal requestAudioPage()
        signal requestSubtitlePage()
        signal requestPlaybackSpeedPage()

        onRequestBack: root.requestClose()
        onRequestAudioPage: tabs.currentIndex = 2
        onRequestSubtitlePage: tabs.currentIndex = 1
        onRequestPlaybackSpeedPage: tabs.currentIndex = 0
    }

    SubtitleStyleSettings {
        id: subtitleStyle
    }

    Rectangle {
        anchors.fill: parent
        color: "#10100F"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: tabRow.implicitHeight + VLCStyle.margin_small * 2
            color: "transparent"

            RowLayout {
                id: tabRow

                anchors.fill: parent
                anchors.margins: VLCStyle.margin_small
                spacing: VLCStyle.margin_xxxsmall

                Repeater {
                    model: [qsTr("Tool"), qsTr("Subtitles"), qsTr("Audio"), qsTr("Video")]

                    T.AbstractButton {
                        id: tabButton

                        required property int index
                        required property string modelData

                        Layout.fillWidth: true
                        implicitHeight: VLCStyle.dp(32, VLCStyle.scale)

                        onClicked: tabs.currentIndex = index

                        contentItem: T.Label {
                            text: tabButton.modelData
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabs.currentIndex === tabButton.index ? "#FFFFFF" : "#8F8F8F"
                            font.pixelSize: VLCStyle.fontSize_normal
                            font.weight: tabs.currentIndex === tabButton.index ? Font.DemiBold : Font.Normal
                        }

                        background: Rectangle {
                            radius: VLCStyle.dp(8, VLCStyle.scale)
                            color: tabs.currentIndex === tabButton.index ? "#2B2927"
                                 : (tabButton.hovered ? "#1B1B1B" : "transparent")
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#2B2927"
        }

        StackLayout {
            id: tabs

            Layout.fillWidth: true
            Layout.fillHeight: true

            Flickable {
                clip: true
                contentHeight: toolColumn.implicitHeight

                ColumnLayout {
                    id: toolColumn

                    x: VLCStyle.margin_normal
                    width: parent.width - VLCStyle.margin_normal * 2
                    anchors.margins: VLCStyle.margin_normal
                    spacing: VLCStyle.margin_large

                    Item { Layout.preferredHeight: VLCStyle.margin_normal }

                    Widgets.SubtitleLabel {
                        Layout.fillWidth: true
                        text: qsTr("Playback speed")
                        color: theme.fg.primary
                    }

                    Widgets.SliderExt {
                        Layout.fillWidth: true
                        from: 0.25
                        to: 4.0
                        stepSize: 0.05
                        value: Player.rate
                        onMoved: Player.rate = value
                    }

                    T.Label {
                        Layout.fillWidth: true
                        text: qsTr("%1x").arg(Number(Player.rate).toLocaleString(Qt.locale(), 'f', 2))
                        color: theme.fg.secondary
                        font.pixelSize: VLCStyle.fontSize_small
                    }

                    Widgets.SubtitleLabel {
                        Layout.fillWidth: true
                        text: qsTr("Tools")
                        color: theme.fg.primary
                    }

                    Widgets.TextToolButton {
                        Layout.fillWidth: true
                        text: Player.recording ? qsTr("Stop recording") : qsTr("Record")
                        onClicked: Player.recording = !Player.recording
                    }

                    Widgets.TextToolButton {
                        Layout.fillWidth: true
                        text: qsTr("Snapshot")
                        onClicked: Player.snapshot()
                    }

                    Widgets.TextToolButton {
                        Layout.fillWidth: true
                        text: qsTr("Extended settings")
                        onClicked: DialogsProvider.extendedDialog()
                    }
                }
            }

            Flickable {
                clip: true
                contentHeight: subsColumn.implicitHeight

                ColumnLayout {
                    id: subsColumn

                    x: VLCStyle.margin_normal
                    width: parent.width - VLCStyle.margin_normal * 2
                    spacing: VLCStyle.margin_normal

                    Item { Layout.preferredHeight: VLCStyle.margin_normal }

                    Widgets.ComboBoxExt {
                        Layout.fillWidth: true
                        model: subtitleStyle.presets
                        currentIndex: Math.max(0, subtitleStyle.presets.indexOf(subtitleStyle.preset))
                        onActivated: (index) => {
                            if (subtitleStyle.presets[index] !== "New Subtitle Preset")
                                subtitleStyle.preset = subtitleStyle.presets[index]
                        }
                        Accessible.name: qsTr("Subtitle preset")
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: VLCStyle.margin_xsmall

                        Widgets.TextFieldExt {
                            id: newPresetName

                            Layout.fillWidth: true
                            placeholderText: qsTr("New preset name")
                            visible: false
                        }

                        Widgets.TextToolButton {
                            text: qsTr("Save")
                            visible: newPresetName.visible
                            onClicked: {
                                subtitleStyle.saveCurrentAsPreset(newPresetName.text)
                                newPresetName.text = ""
                                newPresetName.visible = false
                            }
                        }
                    }

                    Widgets.SubtitleLabel {
                        Layout.fillWidth: true
                        text: qsTr("Appearance")
                        color: theme.fg.primary
                    }

                    T.Label { text: qsTr("Font"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.TextFieldExt {
                        Layout.fillWidth: true
                        text: subtitleStyle.fontFamily
                        onEditingFinished: subtitleStyle.fontFamily = text
                        Accessible.name: qsTr("Subtitle font")
                    }

                    T.Label { text: qsTr("Text color"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.TextFieldExt {
                        Layout.fillWidth: true
                        text: subtitleStyle.textColor.name()
                        onEditingFinished: subtitleStyle.textColor = text
                        Accessible.name: qsTr("Subtitle text color")
                    }

                    T.Label { text: qsTr("Outline color"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.TextFieldExt {
                        Layout.fillWidth: true
                        text: subtitleStyle.outlineColor.name()
                        onEditingFinished: subtitleStyle.outlineColor = text
                        Accessible.name: qsTr("Subtitle outline color")
                    }

                    T.Label { text: qsTr("Subtitle offset (px)"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.SliderExt {
                        Layout.fillWidth: true
                        from: -150
                        to: 150
                        stepSize: 1
                        value: subtitleStyle.offset
                        onMoved: subtitleStyle.offset = value
                    }

                    T.Label { text: qsTr("Font size"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.SliderExt {
                        Layout.fillWidth: true
                        from: 0
                        to: 4
                        stepSize: 1
                        value: subtitleStyle.fontSize
                        onMoved: subtitleStyle.fontSize = value
                    }

                    T.Label { text: qsTr("Outline thickness"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.SliderExt {
                        Layout.fillWidth: true
                        from: 0
                        to: 6
                        stepSize: 1
                        value: subtitleStyle.outlineThickness
                        onMoved: subtitleStyle.outlineThickness = value
                    }

                    T.Label { text: qsTr("Background opacity"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.SliderExt {
                        Layout.fillWidth: true
                        from: 0
                        to: 255
                        stepSize: 1
                        value: subtitleStyle.backgroundOpacity
                        onMoved: subtitleStyle.backgroundOpacity = value
                    }

                    Widgets.SubtitleLabel {
                        Layout.fillWidth: true
                        text: qsTr("Synchronization")
                        color: theme.fg.primary
                    }

                    TracksPageSubtitle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight
                        trackMenuController: panelController
                        showBackButton: false
                        preferredWidth: VLCStyle.dp(280, VLCStyle.scale)
                    }
                }
            }

            Flickable {
                clip: true
                contentHeight: audioColumn.implicitHeight

                ColumnLayout {
                    id: audioColumn

                    x: VLCStyle.margin_normal
                    width: parent.width - VLCStyle.margin_normal * 2
                    spacing: VLCStyle.margin_normal

                    Item { Layout.preferredHeight: VLCStyle.margin_normal }

                    TracksPageAudio {
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight
                        trackMenuController: panelController
                        showBackButton: false
                        preferredWidth: VLCStyle.dp(280, VLCStyle.scale)
                    }
                }
            }

            Flickable {
                clip: true
                contentHeight: videoColumn.implicitHeight

                ColumnLayout {
                    id: videoColumn

                    x: VLCStyle.margin_normal
                    width: parent.width - VLCStyle.margin_normal * 2
                    spacing: VLCStyle.margin_normal

                    Item { Layout.preferredHeight: VLCStyle.margin_normal }

                    Widgets.SubtitleLabel {
                        Layout.fillWidth: true
                        text: qsTr("Video")
                        color: theme.fg.primary
                    }

                    T.Label { text: qsTr("Aspect ratio"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.ComboBoxExt {
                        Layout.fillWidth: true
                        model: Player.aspectRatio
                        textRole: "display"
                        currentIndex: -1
                        onActivated: model.toggleIndex(currentIndex)
                        Accessible.name: qsTr("Aspect ratio")
                    }

                    T.Label { text: qsTr("Crop"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.ComboBoxExt {
                        Layout.fillWidth: true
                        model: Player.crop
                        textRole: "display"
                        currentIndex: -1
                        onActivated: model.toggleIndex(currentIndex)
                        Accessible.name: qsTr("Crop")
                    }

                    T.Label { text: qsTr("Zoom"); color: theme.fg.secondary; font.pixelSize: VLCStyle.fontSize_small }

                    Widgets.ComboBoxExt {
                        Layout.fillWidth: true
                        model: Player.zoom
                        textRole: "display"
                        currentIndex: -1
                        onActivated: model.toggleIndex(currentIndex)
                        Accessible.name: qsTr("Zoom")
                    }
                }
            }
        }
    }
}
