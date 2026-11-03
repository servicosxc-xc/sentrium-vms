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

    function openRecorder(deviceData, extra) {
        if (deviceData.state !== "Connected" || !deviceData.channels.length)
            return
        const n = Math.min(64, deviceData.channels.length)
        if (n > mosaicWorkspace.capacity) {
            const grids = [1, 4, 9, 16, 25, 36, 49, 64]
            for (let i = 0; i < grids.length; ++i) {
                if (grids[i] >= n) {
                    mosaicWorkspace.applyLayout(grids[i], "grid")
                    break
                }
            }
        }
        deviceController.clearAllLive()
        for (let i = 0; i < n; ++i)
            deviceController.openLiveInSlot(
                deviceData.id, deviceData.channels[i].index, i, extra !== false)
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
            deviceGroupChoice.currentIndex = 0
            deviceGroupChoice.editText = "SEM GRUPO"
            deviceName.forceActiveFocus()
        }

        onAccepted: {
            if (connectionMethod.currentIndex === 0) {
                deviceController.addAndConnect(
                    deviceName.text,
                    deviceHost.text,
                    devicePort.value,
                    deviceUser.text,
                    devicePassword.text,
                    deviceGroupChoice.editText)
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
                    registrationPassword.text,
                    deviceGroupChoice.editText)
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

            Label {
                text: "Grupo / local"
                color: window.textSecondary
                font.pixelSize: 11
            }

            ComboBox {
                id: deviceGroupChoice
                Layout.fillWidth: true
                editable: true
                model: ["SEM GRUPO"].concat(deviceController.deviceGroups)
                currentIndex: 0
                selectTextByMouse: true
                onActivated: function(index) {
                    if (index >= 0)
                        editText = model[index]
                }
                // Accept an existing case-insensitive name or a new group.
                // Uniqueness and membership are enforced atomically in C++.
                ToolTip.visible: hovered
                ToolTip.text: "Selecione um grupo existente ou digite outro. "
                    + "Não são permitidos grupos duplicados."
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

    // A single compact top bar: product identity -> scrollable workspaces -> status.
    // Closing a tab never changes the C++ device session ownership.
    header: ToolBar {
        height: 58
        background: Rectangle {
            color: window.panelColor
            border.color: window.borderColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
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
                // Keep the branding at its natural width. Otherwise the outer
                // RowLayout can stretch this column and push tabs to the right.
                Layout.preferredWidth: 187
                Layout.minimumWidth: 187
                Layout.maximumWidth: 187
                Layout.fillWidth: false
                spacing: 1
                Label {
                    text: sentriumProduct
                    color: window.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Label {
                    text: "Intelligent Video Management System"
                    color: window.textSecondary
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 28
                color: window.borderColor
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumWidth: 125
                Layout.preferredHeight: 48
                clip: true

                Flickable {
                    id: headerTabs
                    anchors.fill: parent
                    contentWidth: headerTabsRow.implicitWidth
                    contentHeight: height
                    boundsBehavior: Flickable.StopAtBounds
                    flickableDirection: Flickable.HorizontalFlick
                    clip: true
                    ScrollBar.horizontal: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOff }

                    Row {
                        id: headerTabsRow
                        height: parent.height
                        spacing: 3

                        Repeater {
                            model: window.openModules
                            delegate: Rectangle {
                                id: moduleTab
                                required property string modelData
                                property string moduleName: modelData
                                height: 39
                                width: Math.max(76, Math.min(174,
                                       tabTitle.implicitWidth + (moduleName === "Home" ? 27 : 54)))
                                radius: 5
                                color: window.activeModule === moduleName
                                       ? window.panelAltColor : "transparent"
                                border.color: window.activeModule === moduleName
                                              ? window.accent : "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 9
                                    anchors.rightMargin: 3
                                    spacing: 1
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
                                        id: closeTab
                                        visible: moduleTab.moduleName !== "Home"
                                        implicitWidth: 24
                                        implicitHeight: 27
                                        padding: 0
                                        hoverEnabled: true
                                        background: Item {}
                                        contentItem: Label {
                                            text: "×"
                                            color: closeTab.hovered ? "#ffaeae" : window.textSecondary
                                            font.pixelSize: 17
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        onClicked: window.closeModule(moduleTab.moduleName)
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    anchors.rightMargin: moduleTab.moduleName === "Home" ? 0 : 27
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: window.openModule(moduleTab.moduleName)
                                }
                            }
                        }

                    }
                }
            }

            Button {
                id: settingsButton
                text: "Configurações"
                implicitWidth: 103
                implicitHeight: 31
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
                font.pixelSize: 11
            }

            Rectangle {
                width: 1
                height: 22
                color: window.borderColor
            }

            Label {
                text: sentriumVersion
                color: window.textSecondary
                font.pixelSize: 10
            }
        }
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            HomeDashboard {
                id: homeScreen
                anchors.fill: parent
                visible: window.activeModule === "Home"
                devices: deviceController.devices
                registrationListening: deviceController.registrationListening
                panelColor: window.panelColor
                borderColor: window.borderColor
                textPrimary: window.textPrimary
                textSecondary: window.textSecondary
                accent: window.accent
                onNavigate: function(moduleName) { window.openModule(moduleName) }
            }

            DevicesWorkspace {
                id: devicesModule
                anchors.fill: parent
                visible: window.activeModule === "Dispositivos"
                devices: deviceController.devices
                textPrimary: window.textPrimary
                textSecondary: window.textSecondary
                panelColor: window.panelColor
                panelAltColor: window.panelAltColor
                borderColor: window.borderColor
                accent: window.accent

                onAddDevice: addDeviceDialog.open()
                onConnectDevice: function(deviceId) { deviceController.connectDevice(deviceId) }
                onDisconnectDevice: function(deviceId) { deviceController.disconnectDevice(deviceId) }
                onRemoveDevice: function(deviceId) { deviceController.removeDevice(deviceId) }
                onOpenCamera: function(deviceId, channelIndex, extra) {
                    let tabs = window.openModules.filter(function(name) {
                        return name.indexOf("Visualização") === 0
                    })
                    if (tabs.length > 0)
                        window.activeModule = tabs[0]
                    else
                        window.newVisualization()
                    window.openCamera(deviceId, channelIndex, extra)
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
            SplitView.preferredWidth: 268
            SplitView.minimumWidth: 205
            color: window.panelColor
            border.color: window.borderColor

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                DeviceExplorer {
                    id: deviceExplorer
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 8
                    devices: deviceController.devices
                    groupNames: deviceController.deviceGroups
                    mosaicSlots: deviceController.mosaicSlots
                    liveEventSelections: mosaicWorkspace.liveEventSelections
                    textPrimary: window.textPrimary
                    textSecondary: window.textSecondary
                    accent: window.accent
                    borderColor: window.borderColor
                    panelAltColor: window.panelAltColor
                    onCreateGroup: function(groupName) { deviceController.createDeviceGroup(groupName) }
                    onMoveDevice: function(deviceId, groupName) {
                        deviceController.setDeviceGroup(deviceId, groupName)
                    }
                    onToggleLiveEvents: function(deviceId, channelIndex) {
                        mosaicWorkspace.toggleLiveEvents(deviceId, channelIndex)
                    }
                    onOpenCamera: function(deviceId, channelIndex, extra) {
                        window.openCamera(deviceId, channelIndex, extra)
                    }
                    onOpenRecorder: function(deviceData, extra) {
                        window.openRecorder(deviceData, extra)
                    }
                    onCloseDeviceChannels: function(deviceId) {
                        deviceController.closeDeviceChannels(deviceId)
                    }
                    onConnectDevice: function(deviceId) { deviceController.connectDevice(deviceId) }
                    onDisconnectDevice: function(deviceId) { deviceController.disconnectDevice(deviceId) }
                    onRemoveDevice: function(deviceId) { deviceController.removeDevice(deviceId) }
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
                        property string layoutMode: "grid"
                        property int focusedSlot: -1
                        property int tileGap: 2
                        property int draggedTileSource: -1
                        property int hoveredTileTarget: -1
                        property real dragStartScreenX: 0
                        property real dragStartScreenY: 0
                        property bool dragActivated: false
                        // Selection only: does not enable Server IA or create events.
                        property var liveEventSelections: ({})
                        function liveEventKey(deviceId, channel) { return deviceId + ":" + channel }
                        function isLiveEventsSelected(deviceId, channel) {
                            return liveEventSelections[liveEventKey(deviceId, channel)] === true
                        }
                        function toggleLiveEvents(deviceId, channel) {
                            if (!deviceId || channel < 0) return
                            const key = liveEventKey(deviceId, channel)
                            const next = Object.assign({}, liveEventSelections)
                            if (next[key]) delete next[key]
                            else next[key] = true
                            liveEventSelections = next
                        }

                        property int rightColumns: capacity <= 6 ? 2 : capacity <= 10 ? 3 : 4
                        property int columns: layoutMode === "grid"
                            ? Math.round(Math.sqrt(capacity)) : rightColumns + 2
                        property int rows: layoutMode === "grid"
                            ? columns : Math.ceil((capacity - 1) / rightColumns)

                        function cellFor(slot) {
                            if (layoutMode === "grid")
                                return {col: slot % columns, row: Math.floor(slot / columns),
                                        colSpan: 1, rowSpan: 1}
                            if (slot === 0)
                                return {col: 0, row: 0, colSpan: 2, rowSpan: rows}
                            const offset = slot - 1
                            return {col: 2 + offset % rightColumns,
                                    row: Math.floor(offset / rightColumns),
                                    colSpan: 1, rowSpan: 1}
                        }

                        function dropRecorder(deviceId, startSlot) {
                            if (startSlot < 0 || startSlot >= capacity) return
                            const records = deviceController.devices
                            let device = null
                            for (let i = 0; i < records.length; ++i) {
                                if (records[i].id === deviceId) {
                                    device = records[i]
                                    break
                                }
                            }
                            if (device === null || device.state !== "Connected"
                                || !device.channels || device.channels.length === 0)
                                return
                            const numberOfCameras = Math.min(64, device.channels.length)
                            // Include all cameras: grow to the smallest grid if the
                            // requested contiguous range fits. At slot 63, wrap to
                            // the beginning rather than silently dropping cameras.
                            const required = Math.min(64, startSlot + numberOfCameras)
                            if (required > capacity || numberOfCameras > capacity) {
                                const grids = [1, 4, 9, 16, 25, 36, 49, 64]
                                for (let j = 0; j < grids.length; ++j) {
                                    if (grids[j] >= required
                                        && grids[j] >= numberOfCameras) {
                                        applyLayout(grids[j], "grid")
                                        break
                                    }
                                }
                            }
                            for (let c = 0; c < numberOfCameras; ++c) {
                                const target = (startSlot + c) % capacity
                                deviceController.openLiveInSlot(
                                    deviceId, device.channels[c].index, target, true)
                            }
                            window.activeMosaicSlot = -1
                        }

                        function acceptTreeOrTileDrop(event, targetSlot) {
                            const source = event.source
                            if (!source || targetSlot < 0 || targetSlot >= capacity)
                                return
                            if (source.payloadKind === "camera") {
                                deviceController.openLiveInSlot(
                                    source.payloadDeviceId, source.payloadChannel, targetSlot, true)
                                window.activeMosaicSlot = -1
                                event.acceptProposedAction()
                            } else if (source.payloadKind === "recorder") {
                                dropRecorder(source.payloadDeviceId, targetSlot)
                                event.acceptProposedAction()
                            } else if (source.payloadKind === "tile") {
                                if (source.payloadSourceSlot !== targetSlot) {
                                    deviceController.swapLiveSlots(
                                        source.payloadSourceSlot, targetSlot)
                                }
                                event.acceptProposedAction()
                            }
                        }

                        // Compute the destination from actual tile coordinates.
                        // This keeps tile reordering independent of DropArea/
                        // WindowContainer child HWND drag delivery.
                        function tileAt(x, y) {
                            if (x < 0 || y < 0 || x >= videoMosaic.width
                                || y >= videoMosaic.height)
                                return -1
                            for (let i = 0; i < capacity; ++i) {
                                const tile = mosaicRepeater.itemAt(i)
                                if (tile && tile.visible
                                    && x >= tile.x && x < tile.x + tile.width
                                    && y >= tile.y && y < tile.y + tile.height)
                                    return i
                            }
                            return -1
                        }

                        function beginTileDrag(sourceSlot) {
                            if (sourceSlot < 0 || sourceSlot >= capacity
                                || !slots[sourceSlot] || !slots[sourceSlot].occupied)
                                return
                            const cursor = deviceController.cursorScreenPos()
                            draggedTileSource = sourceSlot
                            dragStartScreenX = cursor.x
                            dragStartScreenY = cursor.y
                            dragActivated = false
                            hoveredTileTarget = -1
                        }

                        function resetTileDrag() {
                            draggedTileSource = -1
                            hoveredTileTarget = -1
                            dragActivated = false
                        }

                        // Poll the actual desktop pointer while dragging. MouseArea
                        // release and DropArea events are not reliable when a
                        // native NetSDK QWindow takes the pointer under the tile.
                        Timer {
                            interval: 16
                            running: mosaicWorkspace.draggedTileSource >= 0
                            repeat: true
                            onTriggered: {
                                const cursor = deviceController.cursorScreenPos()
                                const dx = cursor.x - mosaicWorkspace.dragStartScreenX
                                const dy = cursor.y - mosaicWorkspace.dragStartScreenY
                                if (dx * dx + dy * dy >= 64)
                                    mosaicWorkspace.dragActivated = true
                                const local = videoMosaic.mapFromGlobal(cursor.x, cursor.y)
                                const destination = mosaicWorkspace.tileAt(local.x, local.y)
                                mosaicWorkspace.hoveredTileTarget =
                                    mosaicWorkspace.dragActivated ? destination : -1
                                if (!deviceController.isLeftMouseButtonDown()) {
                                    const source = mosaicWorkspace.draggedTileSource
                                    const moved = mosaicWorkspace.dragActivated
                                    mosaicWorkspace.resetTileDrag()
                                    if (moved && destination >= 0 && destination !== source)
                                        deviceController.swapLiveSlots(source, destination)
                                }
                            }
                        }

                        function applyLayout(count, mode) {
                            if (count === capacity && mode === layoutMode) return
                            // Resize native slots before creating more QML tiles.
                            deviceController.setMosaicCapacity(count)
                            capacity = count
                            layoutMode = mode
                            focusedSlot = -1
                            if (window.activeMosaicSlot >= count)
                                window.activeMosaicSlot = -1
                        }

                        MosaicLayoutsPopup {
                            id: allLayoutsPopup
                            parent: mosaicWorkspace
                            x: Math.max(4, Math.min(210, mosaicWorkspace.width - width - 8))
                            y: 41
                            selectedCapacity: mosaicWorkspace.capacity
                            selectedMode: mosaicWorkspace.layoutMode
                            textPrimary: window.textPrimary
                            textSecondary: window.textSecondary
                            borderColor: window.borderColor
                            panelColor: window.panelColor
                            accent: window.accent
                            onLayoutSelected: function(count, mode) {
                                mosaicWorkspace.applyLayout(count, mode)
                            }
                        }

                        function openTileContextAt(slot, x, y) {
                            if (slot < 0 || slot >= capacity) return
                            cameraContextMenu.targetSlot = slot
                            const popupX = Math.max(0, Math.min(x,
                                mosaicWorkspace.width - cameraContextMenu.width - 6))
                            const popupY = Math.max(0, Math.min(y,
                                mosaicWorkspace.height - 390))
                            cameraContextMenu.popup(popupX, popupY)
                        }

                        function openTileContext(slot, source, localX, localY) {
                            const pos = source.mapToItem(mosaicWorkspace, localX, localY)
                            openTileContextAt(slot, pos.x, pos.y)
                        }

                        Connections {
                            target: deviceController
                            function onVideoSurfaceContextRequested(slot, x, y) {
                                const pos = mosaicWorkspace.mapFromGlobal(x, y)
                                mosaicWorkspace.openTileContextAt(slot, pos.x, pos.y)
                            }
                            function onVideoSurfaceLeftPressed(slot) {
                                mosaicWorkspace.beginTileDrag(slot)
                                window.activeMosaicSlot = slot
                            }
                        }

                        CameraContextMenu {
                            id: cameraContextMenu
                            parent: mosaicWorkspace
                            width: 292
                            slots: deviceController.mosaicSlots
                            focusedSlot: mosaicWorkspace.focusedSlot
                            favorite: hasCamera && deviceExplorer.isFavorite(tile.deviceId, tile.channel)
                            liveEventsSelected: hasCamera && mosaicWorkspace.isLiveEventsSelected(
                                tile.deviceId, tile.channel)
                            liveEventsFeedConnected: false


                            onCloseChannel: function(slot) {
                                deviceController.clearLiveSlot(slot)
                            }
                            onCloseAllChannels: {
                                deviceController.clearAllLive()
                                window.activeMosaicSlot = -1
                            }
                            onSelectTile: function(slot) {
                                window.activeMosaicSlot = slot
                            }
                            onToggleMaximize: function(slot) {
                                window.activeMosaicSlot = slot
                                mosaicWorkspace.focusedSlot =
                                    mosaicWorkspace.focusedSlot === slot ? -1 : slot
                            }
                            onSetStream: function(slot, extra) {
                                if (slot < 0 || slot >= deviceController.mosaicSlots.length) return
                                const current = deviceController.mosaicSlots[slot]
                                if (!current.occupied || current.channel < 0) return
                                deviceController.openLiveInSlot(
                                    current.deviceId, current.channel, slot, extra)
                            }
                            onToggleFavorite: function(deviceId, channelIndex) {
                                deviceExplorer.toggleFavorite(deviceId, channelIndex)
                            }
                            onToggleLiveEvents: function(deviceId, channelIndex) {
                                mosaicWorkspace.toggleLiveEvents(deviceId, channelIndex)
                            }
                        }

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
                                            highlighted: mosaicWorkspace.layoutMode === "grid"
                                                && mosaicWorkspace.capacity === modelData
                                            onClicked: mosaicWorkspace.applyLayout(modelData, "grid")
                                        }
                                    }
                                    Button {
                                        text: "▦  Todos os layouts  ▾"
                                        implicitHeight: 30
                                        onClicked: allLayoutsPopup.open()
                                    }
                                    Item { Layout.fillWidth: true }
                                    Button {
                                        text: "Restaurar grade"
                                        visible: mosaicWorkspace.focusedSlot >= 0
                                        onClicked: mosaicWorkspace.focusedSlot = -1
                                    }
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

                                Item {
                                    id: videoMosaic
                                    anchors.fill: parent
                                    property real cellWidth: Math.max(0,
                                        (width - mosaicWorkspace.tileGap * (mosaicWorkspace.columns - 1))
                                        / mosaicWorkspace.columns)
                                    property real cellHeight: Math.max(0,
                                        (height - mosaicWorkspace.tileGap * (mosaicWorkspace.rows - 1))
                                        / mosaicWorkspace.rows)

                                    Repeater {
                                        id: mosaicRepeater
                                        model: mosaicWorkspace.capacity
                                        delegate: Rectangle {
                                            id: mosaicTile
                                            required property int index
                                            property var itemData: mosaicWorkspace.slots[index]
                                            property bool isActiveTile: window.activeMosaicSlot === index
                                            property var cell: mosaicWorkspace.cellFor(index)
                                            visible: mosaicWorkspace.focusedSlot < 0
                                                || mosaicWorkspace.focusedSlot === index
                                            x: mosaicWorkspace.focusedSlot >= 0 ? 0
                                               : cell.col * (videoMosaic.cellWidth + mosaicWorkspace.tileGap)
                                            y: mosaicWorkspace.focusedSlot >= 0 ? 0
                                               : cell.row * (videoMosaic.cellHeight + mosaicWorkspace.tileGap)
                                            width: mosaicWorkspace.focusedSlot >= 0
                                                ? videoMosaic.width
                                                : cell.colSpan * videoMosaic.cellWidth
                                                  + (cell.colSpan - 1) * mosaicWorkspace.tileGap
                                            height: mosaicWorkspace.focusedSlot >= 0
                                                ? videoMosaic.height
                                                : cell.rowSpan * videoMosaic.cellHeight
                                                  + (cell.rowSpan - 1) * mosaicWorkspace.tileGap
                                            color: "#090e15"
                                            border.color: tileDropArea.containsDrag
                                                || mosaicWorkspace.hoveredTileTarget === index
                                                ? "#70baff" : isActiveTile ? window.accent : "#344354"
                                            border.width: isActiveTile ? 2 : 1
                                            clip: true

                                            Rectangle {
                                                id: tileHeader
                                                height: mosaicWorkspace.capacity > 16
                                                    && mosaicWorkspace.focusedSlot < 0 ? 18 : 24
                                                anchors.top: parent.top
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                color: "#101a27"
                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.leftMargin: 7
                                                    anchors.rightMargin: 7
                                                    spacing: 5
                                                    Label {
                                                        Layout.fillWidth: true
                                                        Layout.minimumWidth: 10
                                                        text: mosaicTile.itemData.occupied
                                                            ? mosaicTile.itemData.channelName
                                                              + " — " + mosaicTile.itemData.deviceName
                                                            : "Canal " + String(index + 1).padStart(2, "0")
                                                        color: window.textPrimary
                                                        font.pixelSize: 10
                                                        font.bold: true
                                                        elide: Text.ElideRight
                                                    }
                                                    Rectangle {
                                                        id: intelligenceBadge
                                                        readonly property bool active:
                                                            mosaicTile.itemData.occupied
                                                            && mosaicTile.itemData.aiState === "Active"
                                                        visible: active
                                                        implicitWidth: active ? aiBadgeLabel.implicitWidth + 10 : 0
                                                        implicitHeight: 17
                                                        radius: 3
                                                        color: mosaicTile.itemData.aiMode === "Motion"
                                                            ? "#C98A28" : "#7A5AF8"
                                                        Label {
                                                            id: aiBadgeLabel
                                                            anchors.centerIn: parent
                                                            text: mosaicTile.itemData.aiMode === "Motion"
                                                                ? "IA · MOV"
                                                                : mosaicTile.itemData.aiMode === "PersonVehicle"
                                                                  ? "IA · P+V" : "IA"
                                                            color: "white"
                                                            font.pixelSize: 9
                                                            font.bold: true
                                                        }
                                                        HoverHandler { id: aiBadgeHover }
                                                        ToolTip.visible: aiBadgeHover.hovered
                                                        ToolTip.text: "Inteligência ativa confirmada pelo Server"
                                                    }
                                                    Rectangle {
                                                        id: liveEventsBadge
                                                        readonly property bool selected:
                                                            mosaicTile.itemData.occupied
                                                            && mosaicWorkspace.isLiveEventsSelected(
                                                                mosaicTile.itemData.deviceId,
                                                                mosaicTile.itemData.channel)
                                                        visible: selected
                                                        implicitWidth: selected
                                                            ? liveEventsBadgeLabel.implicitWidth + 10 : 0
                                                        implicitHeight: 17
                                                        radius: 3
                                                        color: "#2F8CFF"
                                                        Label {
                                                            id: liveEventsBadgeLabel
                                                            anchors.centerIn: parent
                                                            text: mosaicTile.width < 180 ? "EV" : "EVENTOS"
                                                            color: "white"
                                                            font.pixelSize: 9
                                                            font.bold: true
                                                        }
                                                        HoverHandler { id: liveEventsBadgeHover }
                                                        ToolTip.visible: liveEventsBadgeHover.hovered
                                                        ToolTip.text: "Canal selecionado para Eventos ao vivo. "
                                                            + "O feed de eventos ainda não está integrado."
                                                    }
                                                    Rectangle {
                                                        visible: mosaicTile.itemData.occupied
                                                        implicitWidth: visible ? streamLabel.implicitWidth + 9 : 0
                                                        implicitHeight: 16
                                                        radius: 3
                                                        color: "#18222D"
                                                        Label {
                                                            id: streamLabel
                                                            anchors.centerIn: parent
                                                            text: mosaicTile.itemData.extra ? "Extra" : "Principal"
                                                            color: "#9bc3ff"
                                                            font.pixelSize: 9
                                                        }
                                                    }
                                                }
                                                // Header and native video use the same controller
                                                // pointer tracking. A Windows HWND can steal QML
                                                // mouse release, so finishing the drag is timer-driven.
                                                MouseArea {
                                                    anchors.fill: parent
                                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                                    preventStealing: true
                                                    hoverEnabled: true
                                                    cursorShape: mosaicTile.itemData.occupied
                                                        ? Qt.OpenHandCursor : Qt.ArrowCursor
                                                    onPressed: function(mouse) {
                                                        if (mouse.button === Qt.LeftButton)
                                                            mosaicWorkspace.beginTileDrag(mosaicTile.index)
                                                    }
                                                    onClicked: function(mouse) {
                                                        if (mouse.button === Qt.RightButton)
                                                            mosaicWorkspace.openTileContext(
                                                                mosaicTile.index, tileHeader, mouse.x, mouse.y)
                                                        else
                                                            window.activeMosaicSlot = mosaicTile.index
                                                    }
                                                    onDoubleClicked: function(mouse) {
                                                        if (mouse.button !== Qt.LeftButton) return
                                                        window.activeMosaicSlot = mosaicTile.index
                                                        mosaicWorkspace.focusedSlot =
                                                            mosaicWorkspace.focusedSlot === mosaicTile.index
                                                            ? -1 : mosaicTile.index
                                                    }
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
                                                // A native QWindow is attached only for live tiles,
                                                // rather than creating 64 HWNDs for empty positions.
                                                Loader {
                                                    anchors.fill: parent
                                                    active: mosaicTile.itemData.occupied
                                                    sourceComponent: Component {
                                                        WindowContainer {
                                                            anchors.fill: parent
                                                            window: deviceController.liveSurface(mosaicTile.index)
                                                        }
                                                    }
                                                }

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "Clique em um canal para abrir"
                                                    color: window.textSecondary
                                                    font.pixelSize: 10
                                                    visible: !mosaicTile.itemData.occupied
                                                }

                                                // Right click overlay, where supported by the native
                                                // WindowContainer. Header context access is always present.
                                                MouseArea {
                                                    anchors.fill: parent
                                                    acceptedButtons: Qt.RightButton
                                                    onClicked: function(mouse) {
                                                        mosaicWorkspace.openTileContext(
                                                            index, videoViewport, mouse.x, mouse.y)
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                id: tileFooter
                                                anchors.bottom: parent.bottom
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                height: mosaicWorkspace.capacity > 16
                                                    && mosaicWorkspace.focusedSlot < 0 ? 0 : 20
                                                visible: height > 0
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

                                            // Accept drags on any tile, occupied or empty.
                                            // Only the affected slots are reopened/replaced.
                                            DropArea {
                                                id: tileDropArea
                                                anchors.fill: parent
                                                keys: ["sentrium/camera",
                                                       "sentrium/recorder"]
                                                onDropped: function(drop) {
                                                    mosaicWorkspace.acceptTreeOrTileDrop(drop, mosaicTile.index)
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
