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
    title: sentriumProduct + " — Device Connectivity"
    color: "#11151c"

    property color panelColor: "#171c24"
    property color panelAltColor: "#1d2430"
    property color borderColor: "#2b3442"
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    property string runtimeState: "Starting"

    Dialog {
        id: addDeviceDialog
        parent: Overlay.overlay
        modal: true
        width: 440
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        title: "Adicionar dispositivo"
        // Unsupported methods cannot submit fictional connection requests.
        standardButtons: connectionMethod.currentIndex === 1
                         ? Dialog.Cancel
                         : (Dialog.Ok | Dialog.Cancel)

        onOpened: {
            connectionMethod.currentIndex = 0
            deviceName.forceActiveFocus()
        }

        onAccepted: {
            if (connectionMethod.currentIndex === 0) {
                deviceController.addAndConnect(
                    deviceName.text,
                    deviceHost.text,
                    devicePort.value,
                    deviceUser.text,
                    devicePassword.text)
            } else if (connectionMethod.currentIndex === 2) {
                deviceController.addAutoRegistration(
                    deviceName.text,
                    registrationSerial.text,
                    registrationUser.text,
                    registrationPassword.text)
            }
            devicePassword.text = ""
            registrationPassword.text = ""
        }

        onRejected: {
            devicePassword.text = ""
            registrationPassword.text = ""
        }

        ColumnLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Método de conexão"
                color: window.textSecondary
                font.pixelSize: 11
            }

            ComboBox {
                id: connectionMethod
                Layout.fillWidth: true
                model: ["LAN / IP direto", "Cloud / P2P", "Registro Automático"]
                currentIndex: 0
            }

            Label {
                Layout.fillWidth: true
                visible: connectionMethod.currentIndex !== 0
                wrapMode: Text.WordWrap
                color: window.textSecondary
                font.pixelSize: 11
                text: connectionMethod.currentIndex === 1
                      ? "Cloud/P2P exige os binários oficiais do SIM Next e validação do túnel. Ainda indisponível."
                      : "Registro Automático real via NetSDK. Configure o dispositivo para enviar seu número de série para o IP e a porta do SENTRIUM."
            }

            Label {
                visible: connectionMethod.currentIndex === 0
                text: "Intelbras / Dahua NetSDK — IP/LAN"
                color: window.textSecondary
                font.pixelSize: 11
            }

            TextField {
                id: deviceName
                Layout.fillWidth: true
                placeholderText: "Nome do dispositivo"
                text: "NVR Intelbras"
                maximumLength: 128
            }

            TextField {
                id: deviceHost
                visible: connectionMethod.currentIndex === 0
                Layout.fillWidth: true
                placeholderText: "IP ou host"
                selectByMouse: true
            }

            RowLayout {
                visible: connectionMethod.currentIndex === 0
                Layout.fillWidth: true

                Label {
                    text: "Porta"
                    color: window.textSecondary
                }

                SpinBox {
                    id: devicePort
                    from: 1
                    to: 65535
                    value: 37777
                    editable: true
                    Layout.fillWidth: true
                }
            }

            TextField {
                id: deviceUser
                visible: connectionMethod.currentIndex === 0
                Layout.fillWidth: true
                placeholderText: "Usuário"
                selectByMouse: true
            }

            TextField {
                id: devicePassword
                visible: connectionMethod.currentIndex === 0
                Layout.fillWidth: true
                placeholderText: "Senha"
                echoMode: TextInput.Password
                selectByMouse: true
            }

            Label {
                visible: connectionMethod.currentIndex === 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "As credenciais permanecem somente em memória nesta Foundation e não são persistidas."
                color: window.textSecondary
                font.pixelSize: 11
            }

            TextField {
                id: registrationSerial
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                placeholderText: "Número de série exato do gravador"
                maximumLength: 47
                selectByMouse: true
            }

            TextField {
                id: registrationUser
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                placeholderText: "Usuário do gravador"
                selectByMouse: true
            }

            TextField {
                id: registrationPassword
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                placeholderText: "Senha do gravador"
                echoMode: TextInput.Password
                selectByMouse: true
            }

            Label {
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                text: "SERVIDOR DE REGISTRO"
                color: window.textSecondary
                font.pixelSize: 11
                font.bold: true
            }

            RowLayout {
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true

                TextField {
                    id: registrationBind
                    Layout.fillWidth: true
                    text: "127.0.0.1"
                    placeholderText: "IP local do PC"
                    selectByMouse: true
                }

                SpinBox {
                    id: registrationListenPort
                    from: 1
                    to: 65535
                    editable: true
                    value: 9500
                }
            }

            RowLayout {
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: window.textSecondary
                    font.pixelSize: 10
                    text: deviceController.registrationStatus
                }

                Button {
                    text: "Ativar servidor"
                    enabled: !deviceController.registrationListening
                    onClicked: deviceController.startRegistrationServer(
                                   registrationBind.text, registrationListenPort.value)
                }
            }

            Label {
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: window.textSecondary
                font.pixelSize: 10
                text: "Use o IP LAN do PC para receber registros remotos. O SENTRIUM não altera firewall, roteador ou NAT. Credenciais e série ficam apenas em memória."
            }
        }
    }

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
            SplitView.preferredWidth: 310
            SplitView.minimumWidth: 250
            color: window.panelColor
            border.color: window.borderColor

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 14
                    Layout.rightMargin: 10
                    Layout.topMargin: 10
                    Layout.bottomMargin: 8

                    Label {
                        text: "DISPOSITIVOS"
                        color: window.textSecondary
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "+ Adicionar"
                        onClicked: addDeviceDialog.open()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: window.borderColor
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    Layout.topMargin: 5
                    Layout.bottomMargin: 5

                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: deviceController.registrationListening
                              ? "Registro Automático ativo"
                              : "Registro Automático inativo"
                        color: deviceController.registrationListening
                               ? "#7dd3a7" : window.textSecondary
                        font.pixelSize: 10
                    }

                    ToolButton {
                        visible: deviceController.registrationListening
                        text: "Parar"
                        onClicked: deviceController.stopRegistrationServer()
                    }
                }

                Rectangle {
                    visible: deviceController.lastError.length > 0
                    Layout.fillWidth: true
                    implicitHeight: errorLabel.implicitHeight + 16
                    color: "#352126"

                    Label {
                        id: errorLabel
                        anchors.fill: parent
                        anchors.margins: 8
                        text: deviceController.lastError
                        color: "#ffb4b4"
                        wrapMode: Text.WordWrap
                        font.pixelSize: 11
                    }
                }

                Item {
                    visible: deviceController.devices.length === 0
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
                            text: "Adicione um equipamento Intelbras por IP/LAN para validar a primeira integração NetSDK."
                            color: window.textSecondary
                            font.pixelSize: 12
                        }
                    }
                }

                ListView {
                    id: deviceList
                    visible: deviceController.devices.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 8
                    clip: true
                    spacing: 6
                    model: deviceController.devices

                    delegate: Item {
                        id: deviceDelegate
                        width: deviceList.width
                        height: deviceCard.implicitHeight + 2
                        property var deviceData: modelData

                        Rectangle {
                            id: deviceCard
                            width: parent.width
                            implicitHeight: deviceContent.implicitHeight + 20
                            radius: 6
                            color: window.panelAltColor
                            border.color: window.borderColor

                            ColumnLayout {
                                id: deviceContent
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: 10
                                spacing: 5

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        Layout.fillWidth: true
                                        text: deviceDelegate.deviceData.name
                                        color: window.textPrimary
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        text: deviceDelegate.deviceData.state
                                        color: deviceDelegate.deviceData.state === "Connected"
                                               ? "#7dd3a7"
                                               : deviceDelegate.deviceData.state === "Failed"
                                                 ? "#ff9d9d"
                                                 : window.textSecondary
                                        font.pixelSize: 10
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: deviceDelegate.deviceData.connectionLabel
                                    color: window.textSecondary
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }

                                Label {
                                    visible: deviceDelegate.deviceData.diagnostic.length > 0
                                    Layout.fillWidth: true
                                    text: deviceDelegate.deviceData.diagnostic
                                    color: deviceDelegate.deviceData.state === "Failed"
                                           ? "#ffb4b4"
                                           : window.textSecondary
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 10
                                }

                                Repeater {
                                    model: deviceDelegate.deviceData.channels

                                    delegate: RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 7

                                        Rectangle {
                                            width: 6
                                            height: 6
                                            radius: 3
                                            color: "#596579"
                                        }

                                        Label {
                                            text: modelData.name
                                            color: window.textSecondary
                                            font.pixelSize: 11
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Button {
                                        text: "Conectar"
                                        visible: deviceDelegate.deviceData.state === "Disconnected"
                                                 || deviceDelegate.deviceData.state === "Failed"
                                        onClicked: deviceController.connectDevice(deviceDelegate.deviceData.id)
                                    }

                                    Button {
                                        text: "Desconectar"
                                        visible: deviceDelegate.deviceData.state === "Connected"
                                                 || (deviceDelegate.deviceData.method === "AutoRegistration"
                                                     && deviceDelegate.deviceData.state === "Connecting")
                                        onClicked: deviceController.disconnectDevice(deviceDelegate.deviceData.id)
                                    }

                                    Item { Layout.fillWidth: true }

                                    ToolButton {
                                        text: "Remover"
                                        enabled: deviceDelegate.deviceData.state !== "Disconnecting"
                                        onClicked: deviceController.removeDevice(deviceDelegate.deviceData.id)
                                    }
                                }
                            }
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
                                text: "Vídeo ainda não faz parte da Device Foundation."
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
                            text: "Eventos operacionais ainda não fazem parte desta Foundation."
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
