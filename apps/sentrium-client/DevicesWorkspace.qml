import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Devices module: r25-style operational table, not a second connection manager.
// All actions are forwarded to the existing C++ DeviceController.
Rectangle {
    id: workspace
    color: "#0f1319"

    property var devices: []
    property string selectedId: ""
    property color panelColor: "#171c24"
    property color panelAltColor: "#1d2430"
    property color borderColor: "#2b3442"
    property color accent: "#4f8cff"
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"

    signal addDevice()
    signal connectDevice(string deviceId)
    signal disconnectDevice(string deviceId)
    signal removeDevice(string deviceId)
    signal openCamera(string deviceId, int channelIndex, bool extra)

    function selectedDevice() {
        for (let i = 0; i < devices.length; ++i)
            if (devices[i].id === selectedId)
                return devices[i]
        return null
    }

    function countConnected() {
        let count = 0
        for (let i = 0; i < devices.length; ++i)
            if (devices[i].state === "Connected")
                ++count
        return count
    }

    function matchesFilter(device) {
        const term = devicesSearch.text.trim().toLocaleLowerCase()
        return !term || device.name.toLocaleLowerCase().indexOf(term) >= 0
            || device.connectionLabel.toLocaleLowerCase().indexOf(term) >= 0
            || device.method.toLocaleLowerCase().indexOf(term) >= 0
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Label {
                    text: "Dispositivos"
                    color: workspace.textPrimary
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }
                Label {
                    text: "Cadastro, conexões, diagnóstico e canais reais"
                    color: workspace.textSecondary
                    font.pixelSize: 11
                }
            }
            Label {
                text: "Cadastrados: " + workspace.devices.length +
                      "     Conectados: " + workspace.countConnected()
                color: workspace.textSecondary
                font.pixelSize: 11
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            TextField {
                id: devicesSearch
                Layout.fillWidth: true
                Layout.maximumWidth: 355
                placeholderText: "Buscar por nome, conexão ou método"
                font.pixelSize: 12
                selectByMouse: true
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "+ Novo"
                onClicked: workspace.addDevice()
            }
            Button {
                text: "Editar"
                enabled: false
                ToolTip.visible: hovered
                ToolTip.text: "Edição e persistência dependem do contrato de cadastro no C++."
            }
            Button {
                text: "Conectar"
                enabled: workspace.selectedDevice() !== null
                    && (workspace.selectedDevice().state === "Disconnected"
                        || workspace.selectedDevice().state === "Failed")
                onClicked: workspace.connectDevice(workspace.selectedId)
            }
            Button {
                text: "Desconectar"
                enabled: workspace.selectedDevice() !== null
                    && (workspace.selectedDevice().state === "Connected"
                        || (workspace.selectedDevice().method === "AutoRegistration"
                            && workspace.selectedDevice().state === "Connecting"))
                onClicked: workspace.disconnectDevice(workspace.selectedId)
            }
            Button {
                text: "Excluir"
                enabled: workspace.selectedDevice() !== null
                    && workspace.selectedDevice().state !== "Disconnecting"
                onClicked: {
                    const id = workspace.selectedId
                    workspace.selectedId = ""
                    workspace.removeDevice(id)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 33
            color: workspace.panelAltColor
            border.color: workspace.borderColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 11
                anchors.rightMargin: 11
                spacing: 8
                Label {
                    text: "Nome / Endereço"
                    Layout.fillWidth: true
                    color: workspace.textSecondary
                    font.pixelSize: 11
                }
                Label {
                    text: "Método"
                    Layout.preferredWidth: 135
                    color: workspace.textSecondary
                    font.pixelSize: 11
                }
                Label {
                    text: "Canais"
                    Layout.preferredWidth: 64
                    color: workspace.textSecondary
                    font.pixelSize: 11
                }
                Label {
                    text: "Conexão"
                    Layout.preferredWidth: 114
                    color: workspace.textSecondary
                    font.pixelSize: 11
                }
            }
        }

        ListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: workspace.devices

            delegate: Rectangle {
                id: deviceRow
                required property var modelData
                width: deviceList.width
                height: workspace.matchesFilter(modelData) ? 55 : 0
                visible: height > 0
                radius: 4
                color: workspace.selectedId === modelData.id
                    ? "#273449" : workspace.panelColor
                border.color: workspace.selectedId === modelData.id
                    ? workspace.accent : workspace.borderColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 11
                    anchors.rightMargin: 11
                    spacing: 8
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: deviceRow.modelData.name
                            color: workspace.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: deviceRow.modelData.connectionLabel
                            color: workspace.textSecondary
                            font.pixelSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    Label {
                        Layout.preferredWidth: 135
                        text: deviceRow.modelData.method
                        color: workspace.textSecondary
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.preferredWidth: 64
                        text: String(deviceRow.modelData.channels.length)
                        color: workspace.textSecondary
                        font.pixelSize: 11
                    }
                    RowLayout {
                        Layout.preferredWidth: 114
                        spacing: 6
                        Rectangle {
                            implicitWidth: 7
                            implicitHeight: 7
                            radius: 4
                            color: deviceRow.modelData.state === "Connected"
                                ? "#7dd3a7"
                                : deviceRow.modelData.state === "Failed"
                                ? "#ff9191" : "#8492a7"
                        }
                        Label {
                            Layout.fillWidth: true
                            text: deviceRow.modelData.state
                            color: workspace.textSecondary
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: workspace.selectedId = deviceRow.modelData.id
                    onDoubleClicked: workspace.selectedId = deviceRow.modelData.id
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 210
            radius: 5
            color: workspace.panelColor
            border.color: workspace.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 11
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: workspace.selectedDevice() !== null
                            ? workspace.selectedDevice().name + " — canais e diagnóstico"
                            : "Detalhes do dispositivo"
                        color: workspace.textPrimary
                        font.bold: true
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                    Label {
                        text: workspace.selectedDevice() !== null
                            ? workspace.selectedDevice().state : ""
                        color: workspace.textSecondary
                        font.pixelSize: 11
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: workspace.selectedDevice() !== null
                        ? workspace.selectedDevice().diagnostic
                        : "Selecione um dispositivo na tabela para inspecionar os canais."
                    color: workspace.textSecondary
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: workspace.borderColor
                }

                ListView {
                    id: channelList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    model: workspace.selectedDevice() !== null
                        ? workspace.selectedDevice().channels : []

                    delegate: RowLayout {
                        id: channelRow
                        required property var modelData
                        width: channelList.width
                        height: 31
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: channelRow.modelData.name
                            color: workspace.textPrimary
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                        Button {
                            text: "▶ Extra"
                            enabled: workspace.selectedDevice() !== null
                                && workspace.selectedDevice().state === "Connected"
                            onClicked: workspace.openCamera(workspace.selectedId,
                                                             channelRow.modelData.index, true)
                        }
                        Button {
                            text: "▶ Principal"
                            enabled: workspace.selectedDevice() !== null
                                && workspace.selectedDevice().state === "Connected"
                            onClicked: workspace.openCamera(workspace.selectedId,
                                                             channelRow.modelData.index, false)
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: "Os grupos e a edição de dispositivos exigem persistência própria. Nenhuma alteração de credenciais é feita nesta tela."
            color: workspace.textSecondary
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }
    }
}
