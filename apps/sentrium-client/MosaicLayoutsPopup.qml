import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Compact layout palette inspired by the r25 UI. No synthetic video sessions.
Popup {
    id: picker
    width: 412
    padding: 12
    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int selectedCapacity: 16
    property string selectedMode: "grid"
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color panelColor: "#171c24"
    property color borderColor: "#2b3442"
    property color accent: "#4f8cff"

    signal layoutSelected(int capacity, string mode)

    background: Rectangle {
        color: picker.panelColor
        border.color: "#425169"
        radius: 6
    }

    contentItem: ColumnLayout {
        spacing: 9
        Label {
            text: "Grades"
            color: picker.textSecondary
            font.pixelSize: 11
        }
        GridLayout {
            Layout.fillWidth: true
            columns: 4
            rowSpacing: 5
            columnSpacing: 5
            Repeater {
                model: [1, 4, 9, 16, 25, 36, 49, 64]
                delegate: Button {
                    id: gridChoice
                    required property int modelData
                    Layout.fillWidth: true
                    implicitWidth: 88
                    implicitHeight: 54
                    hoverEnabled: true
                    background: Rectangle {
                        radius: 4
                        color: gridChoice.hovered ? "#26374e" : "#101722"
                        border.color: picker.selectedMode === "grid"
                            && picker.selectedCapacity === gridChoice.modelData
                            ? picker.accent : picker.borderColor
                    }
                    contentItem: ColumnLayout {
                        spacing: 1
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: String(gridChoice.modelData)
                            color: picker.textPrimary
                            font.pixelSize: 15
                        }
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: gridChoice.modelData === 1
                                  ? "1 câmera" : gridChoice.modelData + " câmeras"
                            color: picker.textSecondary
                            font.pixelSize: 9
                        }
                    }
                    onClicked: {
                        picker.layoutSelected(modelData, "grid")
                        picker.close()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: picker.borderColor
        }

        Label {
            text: "Destaques"
            color: picker.textSecondary
            font.pixelSize: 11
        }
        GridLayout {
            Layout.fillWidth: true
            columns: 4
            rowSpacing: 5
            columnSpacing: 5
            Repeater {
                model: [6, 8, 10, 12, 14]
                delegate: Button {
                    id: highlightChoice
                    required property int modelData
                    Layout.fillWidth: true
                    implicitWidth: 88
                    implicitHeight: 54
                    hoverEnabled: true
                    background: Rectangle {
                        radius: 4
                        color: highlightChoice.hovered ? "#26374e" : "#101722"
                        border.color: picker.selectedMode === "highlight"
                            && picker.selectedCapacity === highlightChoice.modelData
                            ? picker.accent : picker.borderColor
                    }
                    contentItem: ColumnLayout {
                        spacing: 1
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: String(highlightChoice.modelData)
                            color: picker.textPrimary
                            font.pixelSize: 15
                        }
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Destaque"
                            color: picker.textSecondary
                            font.pixelSize: 9
                        }
                    }
                    onClicked: {
                        picker.layoutSelected(modelData, "highlight")
                        picker.close()
                    }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            text: "25–64 posições: desempenho sujeito à validação com câmeras reais."
            color: picker.textSecondary
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }
    }
}
