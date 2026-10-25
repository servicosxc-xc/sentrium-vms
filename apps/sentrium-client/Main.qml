import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: window
    width: 1440
    height: 900
    minimumWidth: 1080
    minimumHeight: 680
    visible: true
    title: sentriumProduct + " — Application Bootstrap"
    color: "#11151c"

    property color panelColor: "#171c24"
    property color panelAltColor: "#1d2430"
    property color borderColor: "#2b3442"
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    property string runtimeState: "Starting"

    header: ToolBar {
        height: 58

        background: Rectangle {
            color: window.panelColor
            border.color: window.borderColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 14

            Rectangle {
                width: 34
                height: 34
                radius: 8
                color: window.accent

                Label {
                    anchors.centerIn: parent
                    text: "S"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }
            }

            ColumnLayout {
                spacing: 0

                Label {
                    text: sentriumProduct
                    color: window.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }

                Label {
                    text: "Intelligent Video Management System"
                    color: window.textSecondary
                    font.pixelSize: 11
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                text: window.runtimeState
                color: "#7dd3a7"
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 24
                color: window.borderColor
            }

            Label {
                text: sentriumVersion
                color: window.textSecondary
                font.pixelSize: 12
            }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Rectangle {
            SplitView.preferredWidth: 270
            SplitView.minimumWidth: 220
            color: window.panelColor
            border.color: window.borderColor

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Label {
                    text: "DISPOSITIVOS"
                    color: window.textSecondary
                    font.pixelSize: 11
                    font.bold: true
                    leftPadding: 16
                    topPadding: 16
                    bottomPadding: 12
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: window.borderColor
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        anchors.centerIn: parent
                        width: parent.width - 48
                        spacing: 8

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: "Nenhum dispositivo"
                            color: window.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: "A integração com dispositivos será adicionada em uma Foundation posterior."
                            color: window.textSecondary
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            color: "#0f1319"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: workspaceTabs
                    Layout.fillWidth: true

                    background: Rectangle {
                        color: window.panelColor
                        border.color: window.borderColor
                    }

                    TabButton { text: "Visualizar" }
                    TabButton { text: "Reproduzir" }
                }

                StackLayout {
                    currentIndex: workspaceTabs.currentIndex
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        color: "#0f1319"

                        Column {
                            anchors.centerIn: parent
                            spacing: 10

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Área de mosaico"
                                color: window.textPrimary
                                font.pixelSize: 22
                                font.bold: true
                            }

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Nenhum vídeo aberto"
                                color: window.textSecondary
                                font.pixelSize: 13
                            }
                        }
                    }

                    Rectangle {
                        color: "#0f1319"

                        Column {
                            anchors.centerIn: parent
                            spacing: 10

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Reproduzir"
                                color: window.textPrimary
                                font.pixelSize: 22
                                font.bold: true
                            }

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Reprodução ainda não disponível nesta Foundation."
                                color: window.textSecondary
                                font.pixelSize: 13
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: liveEventsPanel
            SplitView.preferredWidth: eventsExpanded ? 310 : 44
            SplitView.minimumWidth: eventsExpanded ? 250 : 44
            color: window.panelColor
            border.color: window.borderColor

            property bool eventsExpanded: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                ToolButton {
                    Layout.fillWidth: true
                    text: liveEventsPanel.eventsExpanded ? "EVENTOS AO VIVO   ›" : "‹"
                    onClicked: liveEventsPanel.eventsExpanded = !liveEventsPanel.eventsExpanded

                    contentItem: Label {
                        text: parent.text
                        color: window.textPrimary
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: liveEventsPanel.eventsExpanded ? Text.AlignLeft : Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: liveEventsPanel.eventsExpanded ? 12 : 0
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: window.borderColor
                }

                Item {
                    visible: liveEventsPanel.eventsExpanded
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        anchors.centerIn: parent
                        width: parent.width - 40
                        spacing: 8

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: "Sem eventos"
                            color: window.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: "Eventos operacionais ainda não fazem parte deste bootstrap."
                            color: window.textSecondary
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }

    footer: Rectangle {
        height: 30
        color: window.panelAltColor
        border.color: window.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Label {
                text: "Status: " + window.runtimeState
                color: window.textSecondary
                font.pixelSize: 11
            }

            Item { Layout.fillWidth: true }

            Label {
                text: sentriumBuildConfiguration + "  •  " + sentriumBuildId
                color: window.textSecondary
                font.pixelSize: 11
            }
        }
    }
}
