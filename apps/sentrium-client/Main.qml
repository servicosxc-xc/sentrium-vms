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

    // Operational shell, modeled on the legacy UX, but implemented natively in Qt.
    // Home is permanent; other modules behave like browser tabs.
    property var openModules: ["Home"]
    property string activeModule: "Home"
    property string selectedDeviceId: ""
    property int visualizationSequence: 0
    property int activeMosaicSlot: -1

    function openCamera(deviceId, channel, extra) {
        if (activeMosaicSlot >= 0) {
            deviceController.openLiveInSlot(deviceId, channel, activeMosaicSlot, extra)
            activeMosaicSlot = -1
        } else {
            deviceController.openLive(deviceId, channel, extra)
        }
    }

    function openRecorder(deviceData) {
        if (deviceData.state !== "Connected" || !deviceData.channels.length)
            return
        const n = Math.min(16, deviceData.channels.length)
        const capacity = n > 9 ? 16 : n > 4 ? 9 : n > 1 ? 4 : 1
        mosaicWorkspace.capacity = capacity
        deviceController.setMosaicCapacity(capacity)
        deviceController.clearAllLive()
        for (let i = 0; i < n; ++i)
            deviceController.openLiveInSlot(
                deviceData.id, deviceData.channels[i].index, i, true)
        activeMosaicSlot = -1
    }

    function newVisualization() {
        visualizationSequence++
        const name = "Visualização " + visualizationSequence
        openModules = openModules.concat([name])
        activeModule = name
    }

    function openModule(name) {
        if (name === "Visualização") {
            newVisualization()
            return
        }
        if (openModules.indexOf(name) === -1)
            openModules = openModules.concat([name])
        activeModule = name
    }

    function closeModule(name) {
        if (name === "Home")
            return
        const items = openModules.filter(function(item) { return item !== name })
        const index = openModules.indexOf(name)
        openModules = items
        if (activeModule === name)
            activeModule = items[Math.max(0, Math.min(index - 1, items.length - 1))]
        if (name.indexOf("Visualização") === 0
            && !items.some(function(item) { return item.indexOf("Visualização") === 0 }))
            deviceController.clearAllLive()
    }

    function connectedCount() {
        const items = deviceController.devices
        let n = 0
        for (let i = 0; i < items.length; i++) {
            if (items[i].state === "Connected")
                n++
        }
        return n
    }

    function selectedDevice() {
        const items = deviceController.devices
        for (let i = 0; i < items.length; i++) {
            if (items[i].id === selectedDeviceId)
                return items[i]
        }
        return null
    }

    Dialog {
        id: addDeviceDialog
        parent: Overlay.overlay
        modal: true
        // A misclick outside the dialog must never discard a device form.
        // Escape and the explicit Cancel button remain available.
        closePolicy: Popup.CloseOnEscape
        width: 440
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        title: "Adicionar dispositivo"
        // Cloud pilot remains blocked after Defender flagged its external
        // bridge binary. Never submit NetSDK credentials to an unknown proxy.
        standardButtons: connectionMethod.currentIndex === 1
                         ? Dialog.Cancel : (Dialog.Ok | Dialog.Cancel)

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
            } else if (connectionMethod.currentIndex === 1) {
                deviceController.addCloudBridgeAndConnect(
                    deviceName.text,
                    cloudLocalPort.value,
                    cloudUser.text,
                    cloudPassword.text)
            } else if (connectionMethod.currentIndex === 2) {
                deviceController.addAutoRegistration(
                    deviceName.text,
                    registrationDeviceId.text,
                    registrationUser.text,
                    registrationPassword.text)
                // The server is managed independently in Configurações.
                // Registering a device does not implicitly open a network port.
            }
            devicePassword.text = ""
            cloudPassword.text = ""
            registrationPassword.text = ""
        }

        onRejected: {
            devicePassword.text = ""
            cloudPassword.text = ""
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
                      ? "Cloud/P2P temporariamente indisponível: o executável auxiliar foi bloqueado pelo Microsoft Defender. Não restaure, não adicione exceções e não informe credenciais em túneis não verificados."
                      : "O ID precisa ser igual ao configurado no gravador. O servidor é ativado separadamente em Configurações > Registro Automático."
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

            Label {
                visible: connectionMethod.currentIndex === 1
                text: "Porta TCP do túnel local (127.0.0.1)"
                color: window.textSecondary
                font.pixelSize: 11
            }

            SpinBox {
                id: cloudLocalPort
                visible: connectionMethod.currentIndex === 1
                enabled: false
                Layout.fillWidth: true
                from: 1024
                to: 65535
                value: 47377
                editable: true
            }

            TextField {
                id: cloudUser
                visible: connectionMethod.currentIndex === 1
                enabled: false
                Layout.fillWidth: true
                placeholderText: "Usuário NetSDK do gravador remoto"
                selectByMouse: true
            }

            TextField {
                id: cloudPassword
                visible: connectionMethod.currentIndex === 1
                enabled: false
                Layout.fillWidth: true
                placeholderText: "Senha NetSDK do gravador remoto"
                echoMode: TextInput.Password
                selectByMouse: true
            }

            Label {
                visible: connectionMethod.currentIndex === 1
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "A implementação de túnel nativo continua em desenvolvimento. O piloto baseado em dh-fwd está suspenso até conclusão da análise de segurança."
                color: window.textSecondary
                font.pixelSize: 10
            }


            TextField {
                id: registrationDeviceId
                visible: connectionMethod.currentIndex === 2
                Layout.fillWidth: true
                placeholderText: "ID de Registro Automático configurado no gravador"
                maximumLength: 31
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

        }
    }

    // Network listener configuration is separate from device enrollment.
    Dialog {
        id: settingsDialog
        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        title: "Configurações — Registro Automático"
        width: 480
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        standardButtons: Dialog.Close

        ColumnLayout {
            width: parent.width
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: "SERVIDOR DE REGISTRO AUTOMÁTICO"
                font.bold: true
                color: window.textPrimary
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "O servidor só aceita registros após ativação manual. Para NVRs externos, utilize o endereço que o gravador consegue alcançar. Esta opção não configura NAT nem firewall."
                color: window.textSecondary
                font.pixelSize: 11
            }

            Label {
                text: "Endereço local para escuta (IPv4)"
                color: window.textSecondary
                font.pixelSize: 11
            }

            TextField {
                id: registrationBind
                Layout.fillWidth: true
                text: "127.0.0.1"
                placeholderText: "Ex.: 192.168.1.100"
                enabled: !deviceController.registrationListening
                selectByMouse: true
            }

            Label {
                text: "Porta de escuta"
                color: window.textSecondary
                font.pixelSize: 11
            }

            SpinBox {
                id: registrationListenPort
                Layout.fillWidth: true
                from: 1
                to: 65535
                value: 9500
                editable: true
                enabled: !deviceController.registrationListening
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "Ativar servidor"
                    enabled: !deviceController.registrationListening
                    onClicked: deviceController.startRegistrationServer(
                                   registrationBind.text.trim(), registrationListenPort.value)
                }

                Button {
                    text: "Parar servidor"
                    enabled: deviceController.registrationListening
                    onClicked: deviceController.stopRegistrationServer()
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: deviceController.registrationListening ? "#7dd3a7" : window.textSecondary
                text: deviceController.registrationStatus
                font.pixelSize: 11
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: window.borderColor
            }

            Label {
                color: window.textPrimary
                text: "Atividade de registro"
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: window.textSecondary
                text: "Eventos recebidos: " + deviceController.registrationEventCount
                font.pixelSize: 11
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: window.textSecondary
                text: deviceController.registrationLastEvent
                font.pixelSize: 11
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: window.textSecondary
                text: "O SENTRIUM não cadastra dispositivos desconhecidos automaticamente. Se um registro chegar com ID diferente, ele será ignorado e aparecerá nesta atividade, sem revelar o ID recebido. As credenciais ficam em memória."
                font.pixelSize: 10
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

            Button {
                id: settingsButton
                text: "Configurações"
                implicitWidth: 112
                implicitHeight: 32
                background: Rectangle {
                    radius: 5
                    color: settingsButton.down ? "#315db0"
                           : settingsButton.hovered ? "#386dc6" : window.panelAltColor
                    border.color: window.borderColor
                }
                contentItem: Label {
                    text: settingsButton.text
                    color: window.textPrimary
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: window.openModule("Configurações")
            }

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#0d121a"
            border.color: window.borderColor

            Row {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 3

                Repeater {
                    model: window.openModules
                    delegate: Rectangle {
                        id: moduleTab
                        property string moduleName: modelData
                        height: 39
                        width: Math.max(96, tabTitle.implicitWidth + (moduleName === "Home" ? 30 : 59))
                        color: window.activeModule === moduleName
                               ? window.panelColor : "#0d121a"
                        border.color: window.activeModule === moduleName
                                      ? window.accent : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 5
                            spacing: 4

                            Label {
                                id: tabTitle
                                Layout.fillWidth: true
                                text: moduleTab.moduleName
                                color: window.activeModule === moduleTab.moduleName
                                       ? window.textPrimary : window.textSecondary
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                            ToolButton {
                                visible: moduleTab.moduleName !== "Home"
                                text: "×"
                                font.pixelSize: 14
                                implicitWidth: 23
                                implicitHeight: 26
                                onClicked: window.closeModule(moduleTab.moduleName)
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            anchors.rightMargin: moduleTab.moduleName === "Home" ? 0 : 27
                            onClicked: window.openModule(moduleTab.moduleName)
                        }
                    }
                }

                ToolButton {
                    text: "+ Nova visualização"
                    height: 38
                    onClicked: window.openModule("Visualização")
                }
                ToolButton {
                    text: "Reprodução"
                    height: 38
                    onClicked: window.openModule("Reprodução")
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Flickable {
                id: homeScreen
                anchors.fill: parent
                visible: window.activeModule === "Home"
                clip: true
                contentWidth: width
                contentHeight: homeLayout.implicitHeight + 60
                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: homeLayout
                    x: 24
                    y: 26
                    width: homeScreen.width - 48
                    spacing: 14

                    Label {
                        text: "Central de operação"
                        color: window.textPrimary
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Label {
                        text: "SENTRIUM VMS  •  Plataforma de gerenciamento de vídeo"
                        color: window.textSecondary
                        font.pixelSize: 12
                    }
                    Label {
                        Layout.topMargin: 16
                        text: "OPERAÇÃO"
                        color: window.textSecondary
                        font.pixelSize: 10
                    }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 4
                        rowSpacing: 10
                        columnSpacing: 10

                        Repeater {
                            model: [
                                { title: "Visualização", glyph: "◉", module: "Visualização" },
                                { title: "Reprodução", glyph: "▶", module: "Reprodução" },
                                { title: "Dispositivos", glyph: "▦", module: "Dispositivos" },
                                { title: "Eventos", glyph: "ϟ", module: "Eventos" },
                                { title: "Regras IA", glyph: "◇", module: "Regras IA" },
                                { title: "Mídia", glyph: "▣", module: "Mídia" },
                                { title: "LPR", glyph: "▤", module: "LPR" },
                                { title: "Configurações", glyph: "⚙", module: "Configurações" }
                            ]

                            delegate: Rectangle {
                                id: homeCard
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 120
                                radius: 5
                                color: cardMouse.containsMouse ? "#253247" : window.panelColor
                                border.color: cardMouse.containsMouse ? window.accent : window.borderColor
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 4
                                    Label {
                                        text: homeCard.modelData.glyph
                                        color: window.accent
                                        font.pixelSize: 19
                                    }
                                    Label {
                                        text: homeCard.modelData.title
                                        color: window.textPrimary
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                    Item { Layout.fillHeight: true }
                                    Label {
                                        Layout.alignment: Qt.AlignRight
                                        text: "Abrir  →"
                                        color: window.accent
                                        font.pixelSize: 11
                                    }
                                }
                                MouseArea {
                                    id: cardMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: window.openModule(homeCard.modelData.module)
                                }
                            }
                        }
                    }
                    Label {
                        Layout.topMargin: 18
                        text: "ESTADO OPERACIONAL"
                        color: window.textSecondary
                        font.pixelSize: 10
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Repeater {
                            model: [
                                { title: "Dispositivos cadastrados", value: String(deviceController.devices.length) },
                                { title: "Sessões conectadas", value: String(window.connectedCount()) },
                                { title: "Registro Automático", value: deviceController.registrationListening ? "Ativo" : "Inativo" },
                                { title: "Cloud / P2P", value: "Em implementação" }
                            ]
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 90
                                color: window.panelColor
                                border.color: window.borderColor
                                radius: 4
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    Label {
                                        text: modelData.title
                                        color: window.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Label {
                                        text: modelData.value
                                        color: window.textPrimary
                                        font.pixelSize: 15
                                        font.bold: true
                                    }
                                }
                            }
                        }
                    }
                    Label {
                        Layout.topMargin: 10
                        text: "Conectividade real por LAN e Registro Automático em evolução. Recursos sem backend permanecem identificados como indisponíveis."
                        color: window.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Rectangle {
                id: devicesModule
                anchors.fill: parent
                visible: window.activeModule === "Dispositivos"
                color: "#0f1319"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Dispositivos"
                            color: window.textPrimary
                            font.bold: true
                            font.pixelSize: 21
                        }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: "Cadastrados: " + deviceController.devices.length
                                  + "    Conectados: " + window.connectedCount()
                            color: window.textSecondary
                            font.pixelSize: 11
                        }
                    }
                    Label {
                        text: "Cadastro, conexão e diagnóstico de gravadores e câmeras"
                        color: window.textSecondary
                        font.pixelSize: 11
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        TextField {
                            id: deviceSearch
                            Layout.preferredWidth: 300
                            placeholderText: "Pesquisar dispositivo"
                        }
                        Item { Layout.fillWidth: true }
                        Button { text: "Novo"; onClicked: addDeviceDialog.open() }
                        Button {
                            text: "Conectar"
                            enabled: window.selectedDevice() !== null
                                     && (window.selectedDevice().state === "Disconnected"
                                         || window.selectedDevice().state === "Failed")
                            onClicked: deviceController.connectDevice(window.selectedDeviceId)
                        }
                        Button {
                            text: "Desconectar"
                            enabled: window.selectedDevice() !== null
                                     && window.selectedDevice().state === "Connected"
                            onClicked: deviceController.disconnectDevice(window.selectedDeviceId)
                        }
                        Button {
                            text: "Excluir"
                            enabled: window.selectedDevice() !== null
                            onClicked: {
                                deviceController.removeDevice(window.selectedDeviceId)
                                window.selectedDeviceId = ""
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        color: window.panelAltColor
                        border.color: window.borderColor
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            Label { text: "Dispositivo"; Layout.fillWidth: true; color: window.textSecondary; font.pixelSize: 11 }
                            Label { text: "Tipo de conexão"; Layout.preferredWidth: 165; color: window.textSecondary; font.pixelSize: 11 }
                            Label { text: "Canais"; Layout.preferredWidth: 90; color: window.textSecondary; font.pixelSize: 11 }
                            Label { text: "Estado"; Layout.preferredWidth: 120; color: window.textSecondary; font.pixelSize: 11 }
                        }
                    }
                    ListView {
                        id: devicesTable
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: deviceController.devices

                        delegate: Rectangle {
                            id: deviceRow
                            property var item: modelData
                            width: devicesTable.width
                            height: (!deviceSearch.text
                                     || item.name.toLowerCase().indexOf(deviceSearch.text.toLowerCase()) >= 0)
                                    ? 48 : 0
                            visible: height > 0
                            radius: 3
                            color: window.selectedDeviceId === item.id ? "#273449" : window.panelColor
                            border.color: window.selectedDeviceId === item.id ? window.accent : window.borderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 8
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0
                                    Label {
                                        text: deviceRow.item.name
                                        color: window.textPrimary
                                        font.pixelSize: 12
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }
                                    Label {
                                        text: deviceRow.item.connectionLabel
                                        color: window.textSecondary
                                        font.pixelSize: 10
                                        elide: Text.ElideRight
                                    }
                                }
                                Label {
                                    Layout.preferredWidth: 165
                                    text: deviceRow.item.method
                                    color: window.textSecondary
                                    font.pixelSize: 11
                                }
                                Label {
                                    Layout.preferredWidth: 90
                                    text: String(deviceRow.item.channels.length)
                                    color: window.textSecondary
                                    font.pixelSize: 11
                                }
                                Label {
                                    Layout.preferredWidth: 120
                                    text: deviceRow.item.state
                                    color: deviceRow.item.state === "Connected"
                                            ? "#7dd3a7" : window.textSecondary
                                    font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: window.selectedDeviceId = deviceRow.item.id
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 146
                        radius: 4
                        color: window.panelColor
                        border.color: window.borderColor
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 7
                            Label {
                                text: "Dispositivo selecionado"
                                color: window.textPrimary
                                font.bold: true
                                font.pixelSize: 12
                            }
                            Label {
                                Layout.fillWidth: true
                                text: window.selectedDevice() !== null
                                      ? window.selectedDevice().name + "  •  " + window.selectedDevice().method
                                        + "  •  " + window.selectedDevice().state
                                      : "Selecione um dispositivo na tabela"
                                color: window.textSecondary
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                            Label {
                                Layout.fillWidth: true
                                text: window.selectedDevice() !== null
                                      ? window.selectedDevice().diagnostic : ""
                                color: window.textSecondary
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                            Label {
                                Layout.fillWidth: true
                                text: window.selectedDevice() !== null
                                      ? "Canais retornados pelo NetSDK: " + window.selectedDevice().channels.length
                                      : ""
                                color: window.textSecondary
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: ["Eventos", "Regras IA", "Mídia", "LPR"].indexOf(window.activeModule) !== -1
                color: "#0f1319"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12
                    Label {
                        text: window.activeModule
                        color: window.textPrimary
                        font.pixelSize: 22
                        font.bold: true
                    }
                    Label {
                        text: window.activeModule === "Eventos"
                            ? "Central de eventos — estrutura preparada para dados operacionais"
                            : window.activeModule === "Regras IA"
                              ? "Gerenciamento de regras por câmera"
                              : window.activeModule === "LPR"
                                ? "Leitura de placas e perfis LPR"
                                : "Fotos, vídeos e evidências"
                        color: window.textSecondary
                        font.pixelSize: 11
                    }
                    TabBar {
                        id: featureTabs
                        Layout.fillWidth: true
                        background: Rectangle { color: window.panelColor }
                        Repeater {
                            model: window.activeModule === "Regras IA"
                                   ? ["Câmeras com regras", "Regras da câmera", "Detalhes"]
                                   : window.activeModule === "LPR"
                                     ? ["Perfis LPR", "Pipeline", "Leituras", "Diagnóstico"]
                                     : window.activeModule === "Eventos"
                                       ? ["Todos", "Pessoa", "Veículo", "LPR", "Movimento"]
                                       : ["Evidências", "Visualizar", "Detalhes"]
                            delegate: TabButton { text: modelData }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: window.panelColor
                        border.color: window.borderColor
                        Column {
                            anchors.centerIn: parent
                            spacing: 9
                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Módulo em desenvolvimento"
                                font.pixelSize: 17
                                color: window.textPrimary
                            }
                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Estrutura de navegação disponível; backend deste módulo ainda não implementado no SENTRIUM."
                                font.pixelSize: 11
                                color: window.textSecondary
                            }
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: window.activeModule === "Configurações"
                color: "#0f1319"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12
                    Label {
                        text: "Configurações"
                        color: window.textPrimary
                        font.pixelSize: 22
                        font.bold: true
                    }
                    Label {
                        text: "Preferências do sistema, conectividade e gerenciamento"
                        color: window.textSecondary
                        font.pixelSize: 11
                    }
                    TabBar {
                        id: settingsTabs
                        Layout.fillWidth: true
                        background: Rectangle { color: window.panelColor }
                        TabButton { text: "Sistema" }
                        TabButton { text: "Armazenamento" }
                        TabButton { text: "Usuários e permissões" }
                        TabButton { text: "Notificações" }
                        TabButton { text: "Integrações" }
                    }
                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: settingsTabs.currentIndex
                        Repeater {
                            model: 5
                            delegate: Rectangle {
                                required property int index
                                color: window.panelColor
                                border.color: window.borderColor
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 12
                                    Label {
                                        text: index === 0 ? "Sistema e conectividade"
                                              : ["", "Armazenamento", "Usuários e permissões", "Notificações", "Integrações"][index]
                                        color: window.textPrimary
                                        font.pixelSize: 15
                                        font.bold: true
                                    }
                                    Label {
                                        visible: index === 0 || index === 4
                                        text: "Registro Automático: " +
                                              (deviceController.registrationListening ? "Ativo" : "Inativo")
                                        color: deviceController.registrationListening
                                               ? "#7dd3a7" : window.textSecondary
                                        font.pixelSize: 11
                                    }
                                    Button {
                                        visible: index === 0 || index === 4
                                        text: "Configurar servidor de Registro Automático"
                                        onClicked: settingsDialog.open()
                                    }
                                    Label {
                                        visible: index !== 0 && index !== 4
                                        text: "Configuração ainda não disponível nesta Foundation."
                                        color: window.textSecondary
                                        font.pixelSize: 11
                                    }
                                    Item { Layout.fillHeight: true }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: window.activeModule === "Reprodução"
                color: "#0f1319"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12
                    Label {
                        text: "Reprodução"
                        color: window.textPrimary
                        font.pixelSize: 23
                        font.bold: true
                    }
                    Label {
                        text: "Pesquisa e reprodução de gravações — módulo independente"
                        color: window.textSecondary
                        font.pixelSize: 12
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 45
                        color: window.panelColor
                        border.color: window.borderColor
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: 10
                            Label { text: "Dispositivo"; color: window.textSecondary }
                            ComboBox {
                                enabled: false
                                model: ["Consulta de gravações pendente"]
                                Layout.preferredWidth: 245
                            }
                            Label { text: "Data / horário"; color: window.textSecondary }
                            TextField {
                                enabled: false
                                placeholderText: "Pesquisar gravação"
                                Layout.fillWidth: true
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#090f16"
                        border.color: window.borderColor
                        Column {
                            anchors.centerIn: parent
                            spacing: 9
                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Player de gravações"
                                color: window.textPrimary
                                font.pixelSize: 20
                            }
                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Timeline e consulta NetSDK ainda não implementadas"
                                color: window.textSecondary
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            SplitView {
                visible: window.activeModule.indexOf("Visualização") === 0
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
                        id: addDeviceButton
                        text: "+ Adicionar"
                        Layout.preferredWidth: 106
                        Layout.preferredHeight: 32
                        padding: 0
                        hoverEnabled: true
                        background: Rectangle {
                            radius: 5
                            color: !addDeviceButton.enabled
                                   ? window.panelAltColor
                                   : addDeviceButton.down
                                     ? "#315db0"
                                     : addDeviceButton.hovered
                                       ? "#386dc6" : window.accent
                            border.color: addDeviceButton.enabled
                                          ? "#6da4ff" : window.borderColor
                        }
                        contentItem: Label {
                            text: addDeviceButton.text
                            color: "white"
                            font.pixelSize: 11
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
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
                        text: "Abrir"
                        onClicked: window.openModule("Configurações")
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
                            text: "Adicione um gravador por LAN ou Registro Automático. Cloud/P2P independente está em desenvolvimento."
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

                                // Cloud status comes from measured tunnel telemetry only.
                                // Unknown never becomes Direct merely because a port exists.
                                Label {
                                    visible: deviceDelegate.deviceData.method === "Cloud"
                                             && deviceDelegate.deviceData.state === "Connected"
                                             && deviceDelegate.deviceData.cloudRoute !== "Unknown"
                                    Layout.fillWidth: true
                                    text: deviceDelegate.deviceData.cloudRoute === "Relay"
                                          ? "Cloud / Relay" : "Cloud / P2P direto"
                                    color: deviceDelegate.deviceData.cloudRoute === "Relay"
                                           ? "#f6cf74" : "#6da4ff"
                                    font.pixelSize: 10
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
                                            Layout.fillWidth: true
                                        }
                                        Button {
                                            text: "▶ Extra"
                                            enabled: deviceDelegate.deviceData.state === "Connected"
                                            onClicked: window.openCamera(
                                                deviceDelegate.deviceData.id, modelData.index, true)
                                            ToolTip.visible: hovered
                                            ToolTip.text: "Abre o canal Extra dentro do mosaico."
                                        }
                                        Button {
                                            text: "▶ Principal"
                                            enabled: deviceDelegate.deviceData.state === "Connected"
                                            onClicked: window.openCamera(
                                                deviceDelegate.deviceData.id, modelData.index, false)
                                            ToolTip.visible: hovered
                                            ToolTip.text: "Abre o canal Principal dentro do mosaico."
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Button {
                                        text: "Abrir canais"
                                        visible: deviceDelegate.deviceData.state === "Connected"
                                        enabled: deviceDelegate.deviceData.channels.length > 0
                                        onClicked: window.openRecorder(deviceDelegate.deviceData)
                                        ToolTip.visible: hovered
                                        ToolTip.text: "Preenche o mosaico com os canais disponíveis (fluxo Extra)."
                                    }

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

                Rectangle {
                    id: mosaicWorkspace
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#0f1319"
                        property var slots: deviceController.mosaicSlots
                        property int capacity: 16
                        property int columns: capacity === 1 ? 1 : capacity === 4 ? 2 : capacity === 9 ? 3 : 4

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 3

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 38
                                color: "#141b25"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 9
                                    anchors.rightMargin: 9
                                    spacing: 4

                                    Label {
                                        text: "Layout"
                                        color: window.textSecondary
                                        font.pixelSize: 11
                                        Layout.rightMargin: 4
                                    }

                                    Repeater {
                                        model: [1, 4, 9, 16]
                                        delegate: Button {
                                            required property int modelData
                                            text: String(modelData)
                                            implicitWidth: 42
                                            implicitHeight: 30
                                            highlighted: mosaicWorkspace.capacity === modelData
                                            onClicked: {
                                                mosaicWorkspace.capacity = modelData
                                                deviceController.setMosaicCapacity(modelData)
                                                if (window.activeMosaicSlot >= modelData)
                                                    window.activeMosaicSlot = -1
                                            }
                                        }
                                    }
                                    Item { Layout.fillWidth: true }
                                    Label {
                                        text: "Selecione um tile para direcionar a câmera"
                                        color: window.textSecondary
                                        font.pixelSize: 10
                                        visible: mosaicWorkspace.capacity > 1
                                    }
                                    Button {
                                        text: "Limpar"
                                        onClicked: {
                                            deviceController.clearAllLive()
                                            window.activeMosaicSlot = -1
                                        }
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true

                                Grid {
                                    id: videoMosaic
                                    anchors.fill: parent
                                    columns: mosaicWorkspace.columns
                                    spacing: 2

                                    Repeater {
                                        model: mosaicWorkspace.capacity
                                        delegate: Rectangle {
                                            id: mosaicTile
                                            required property int index
                                            property var itemData: mosaicWorkspace.slots[index]
                                            property bool isActiveTile: window.activeMosaicSlot === index
                                            width: Math.max(0, (videoMosaic.width -
                                                videoMosaic.spacing * (videoMosaic.columns - 1)) / videoMosaic.columns)
                                            height: Math.max(0, (videoMosaic.height -
                                                videoMosaic.spacing * (videoMosaic.columns - 1)) / videoMosaic.columns)
                                            color: "#090e15"
                                            border.color: isActiveTile ? window.accent : "#344354"
                                            border.width: isActiveTile ? 2 : 1
                                            clip: true

                                            Rectangle {
                                                id: tileHeader
                                                height: 24
                                                anchors.top: parent.top
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                color: "#101a27"
                                                Label {
                                                    anchors.left: parent.left
                                                    anchors.leftMargin: 7
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    width: parent.width - 90
                                                    text: mosaicTile.itemData.occupied
                                                        ? mosaicTile.itemData.channelName +
                                                          " — " + mosaicTile.itemData.deviceName
                                                        : "Canal " + String(index + 1).padStart(2, "0")
                                                    elide: Text.ElideRight
                                                    color: window.textPrimary
                                                    font.pixelSize: 10
                                                }
                                                Label {
                                                    anchors.right: parent.right
                                                    anchors.rightMargin: 9
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    text: mosaicTile.itemData.occupied
                                                          ? (mosaicTile.itemData.extra ? "Extra" : "Principal")
                                                          : ""
                                                    color: "#9bc3ff"
                                                    font.pixelSize: 9
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: window.activeMosaicSlot = index
                                                }
                                            }

                                            Item {
                                                id: videoViewport
                                                anchors.top: tileHeader.bottom
                                                anchors.bottom: tileFooter.top
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                clip: true

                                                // Qt 6.12 WindowContainer owns QWindow position, clipping and
                                                // visibility. NetSDK draws real frames in the child HWND.
                                                WindowContainer {
                                                    anchors.fill: parent
                                                    window: deviceController.liveSurface(index)
                                                    visible: mosaicTile.itemData.occupied
                                                }

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "Clique em um canal para abrir"
                                                    color: window.textSecondary
                                                    font.pixelSize: 10
                                                    visible: !mosaicTile.itemData.occupied
                                                }
                                            }

                                            Rectangle {
                                                id: tileFooter
                                                anchors.bottom: parent.bottom
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                height: 20
                                                color: "#101a27"
                                                Row {
                                                    anchors.left: parent.left
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    anchors.leftMargin: 6
                                                    spacing: 7
                                                    Label {
                                                        text: mosaicTile.itemData.occupied
                                                            ? mosaicTile.itemData.status : "Livre"
                                                        color: window.textSecondary
                                                        font.pixelSize: 9
                                                    }
                                                }
                                                ToolButton {
                                                    anchors.right: parent.right
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    implicitWidth: 26
                                                    implicitHeight: 19
                                                    text: "×"
                                                    enabled: mosaicTile.itemData.occupied
                                                    onClicked: deviceController.clearLiveSlot(index)
                                                }
                                            }
                                        }
                                    }
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
            } // SplitView
        } // module content
    } // application shell

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
