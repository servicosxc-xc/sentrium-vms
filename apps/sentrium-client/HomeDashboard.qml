import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Operational landing page, reconstructed from the r25 HomeView UX contract.
// Navigation only: no simulated backend, device sessions or telemetry.
Flickable {
    id: home
    clip: true
    contentWidth: width
    contentHeight: Math.max(height, page.implicitHeight + 64)
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar {}

    property var devices: []
    property bool registrationListening: false
    property color panelColor: "#171c24"
    property color borderColor: "#2b3442"
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    signal navigate(string moduleName)

    function connectedCount() {
        let count = 0
        for (let i = 0; i < devices.length; ++i)
            if (devices[i].state === "Connected")
                ++count
        return count
    }

    ColumnLayout {
        id: page
        x: 34
        y: 28
        width: Math.max(0, home.width - 68)
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                Label {
                    text: "Central de operação"
                    color: home.textPrimary
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                }
                Label {
                    text: "SENTRIUM VMS  •  Gerenciamento inteligente de vídeo"
                    color: home.textSecondary
                    font.pixelSize: 12
                }
            }

            Rectangle {
                Layout.preferredWidth: 162
                Layout.preferredHeight: 37
                color: home.panelColor
                border.color: home.borderColor
                radius: 6

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Rectangle {
                        width: 7
                        height: 7
                        radius: 4
                        color: "#58b5ff"
                    }
                    Label {
                        text: "Interface disponível"
                        color: home.textSecondary
                        font.pixelSize: 11
                    }
                }
            }
        }

        Label {
            Layout.topMargin: 16
            text: "OPERAÇÃO"
            color: home.textSecondary
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 1.2
        }

        GridLayout {
            id: modulesGrid
            Layout.fillWidth: true
            columns: width >= 940 ? 4 : width >= 620 ? 3 : 2
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { title: "Visualização", detail: "Mosaicos e câmeras ao vivo", glyph: "◉", module: "Visualização", tone: "#4f8cff" },
                    { title: "Dispositivos", detail: "Cadastro, conexão e canais", glyph: "▦", module: "Dispositivos", tone: "#58b5ff" },
                    { title: "Eventos", detail: "Central global de eventos", glyph: "⚡", module: "Eventos", tone: "#f3bf63" },
                    { title: "Regras IA", detail: "Câmeras, ROI e ações", glyph: "◇", module: "Regras IA", tone: "#a889ff" },
                    { title: "Mídia", detail: "Fotos e vídeos de evidências", glyph: "▣", module: "Mídia", tone: "#4ec9b0" },
                    { title: "LPR", detail: "Perfis e leitura de placas", glyph: "▤", module: "LPR", tone: "#ffb75e" },
                    { title: "Configurações", detail: "Sistema e integrações", glyph: "⚙", module: "Configurações", tone: "#9bb7d4" },
                    { title: "Reprodução", detail: "Gravações dos dispositivos", glyph: "▶", module: "Reprodução", tone: "#8ec8ff" }
                ]
                delegate: Rectangle {
                    id: moduleCard
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 143
                    color: moduleHover.containsMouse ? "#253247" : home.panelColor
                    border.color: moduleHover.containsMouse ? home.accent : home.borderColor
                    radius: 7

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 13
                        spacing: 7

                        Rectangle {
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 40
                            color: "#273140"
                            radius: 9
                            Label {
                                anchors.centerIn: parent
                                text: moduleCard.modelData.glyph
                                color: moduleCard.modelData.tone
                                font.pixelSize: 23
                                font.bold: true
                            }
                        }
                        Label {
                            text: moduleCard.modelData.title
                            color: home.textPrimary
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Label {
                            text: moduleCard.modelData.detail
                            color: home.textSecondary
                            font.pixelSize: 11
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    MouseArea {
                        id: moduleHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        accessible.name: "Abrir " + moduleCard.modelData.title
                        onClicked: home.navigate(moduleCard.modelData.module)
                    }
                }
            }
        }

        Label {
            Layout.topMargin: 17
            text: "ESTADO OPERACIONAL"
            color: home.textSecondary
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 1.2
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width >= 750 ? 4 : 2
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { title: "Dispositivos cadastrados", value: String(home.devices.length) },
                    { title: "Sessões conectadas", value: String(home.connectedCount()) },
                    { title: "Registro Automático", value: home.registrationListening ? "Ativo" : "Inativo" },
                    { title: "Cloud / P2P", value: "Bloqueado" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 82
                    color: home.panelColor
                    border.color: home.borderColor
                    radius: 6
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 13
                        spacing: 6
                        Label {
                            text: modelData.title
                            font.pixelSize: 10
                            color: home.textSecondary
                        }
                        Label {
                            text: modelData.value
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            color: home.textPrimary
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 12
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 99
                color: home.panelColor
                border.color: home.borderColor
                radius: 6
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 8
                    Label { text: "INTELIGÊNCIA"; color: home.textSecondary; font.pixelSize: 10; font.bold: true }
                    Label {
                        text: "IA / LPR: processamento ainda não integrado nesta geração."
                        color: home.textPrimary
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 99
                color: home.panelColor
                border.color: home.borderColor
                radius: 6
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 8
                    Label { text: "SERVER"; color: home.textSecondary; font.pixelSize: 10; font.bold: true }
                    Label {
                        text: "Telemetria do Server ainda indisponível."
                        color: home.textPrimary
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Label {
            Layout.topMargin: 15
            Layout.fillWidth: true
            text: "Módulos em preparação: Relatórios · Licenças · Integrações. Recursos sem backend não simulam resultados."
            color: home.textSecondary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }
}
