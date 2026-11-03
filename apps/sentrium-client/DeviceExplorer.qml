import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Hierarchical operator tree: Group/Site -> Recorder -> Camera.
// Tree state/favorites are transient UI state until DataRoot persistence is approved.
// All device sessions and video actions continue through the existing controller.
Rectangle {
    id: explorer
    color: "transparent"

    property var devices: []
    property var groupNames: []
    property var mosaicSlots: []
    property var liveEventSelections: ({})
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    property color borderColor: "#2b3442"
    property color panelAltColor: "#1d2430"
    property var favorites: ({})
    property var expandedGroups: ({})
    property var expandedDevices: ({})
    property bool favoritesOnly: false
    property string ungroupedLabel: "SEM GRUPO"
    readonly property var treeRows: buildTree()

    signal openCamera(string deviceId, int channelIndex, bool extra)
    signal openRecorder(var deviceData, bool extra)
    signal closeDeviceChannels(string deviceId)
    signal connectDevice(string deviceId)
    signal disconnectDevice(string deviceId)
    signal removeDevice(string deviceId)
    signal createGroup(string groupName)
    signal moveDevice(string deviceId, string groupName)
    signal toggleLiveEvents(string deviceId, int channelIndex)

    function deviceGroup(device) {
        return device.group && device.group.length ? device.group : ungroupedLabel
    }

    function cameraKey(deviceId, channelIndex) {
        return deviceId + ":" + channelIndex
    }

    function isLiveEventsSelected(deviceId, channelIndex) {
        return liveEventSelections[cameraKey(deviceId, channelIndex)] === true
    }

    function isFavorite(deviceId, channelIndex) {
        return favorites[cameraKey(deviceId, channelIndex)] === true
    }

    function toggleFavorite(deviceId, channelIndex) {
        const key = cameraKey(deviceId, channelIndex)
        const next = Object.assign({}, favorites)
        if (next[key]) delete next[key]
        else next[key] = true
        favorites = next
    }

    function groupExpanded(groupName) {
        return expandedGroups[groupName] !== false
    }

    function deviceExpanded(id) {
        return expandedDevices[id] === true
    }

    function setGroupExpanded(groupName, expanded) {
        const next = Object.assign({}, expandedGroups)
        next[groupName] = expanded
        expandedGroups = next
    }

    function setDeviceExpanded(id, expanded) {
        const next = Object.assign({}, expandedDevices)
        next[id] = expanded
        expandedDevices = next
    }

    function collapseAll() {
        const nextGroups = {}
        const nextDevices = {}
        const names = availableGroups()
        for (let i = 0; i < names.length; ++i) nextGroups[names[i]] = false
        for (let i = 0; i < devices.length; ++i) nextDevices[devices[i].id] = false
        expandedGroups = nextGroups
        expandedDevices = nextDevices
    }

    function expandAll() {
        const nextGroups = {}
        const nextDevices = {}
        const names = availableGroups()
        for (let i = 0; i < names.length; ++i) nextGroups[names[i]] = true
        for (let i = 0; i < devices.length; ++i) nextDevices[devices[i].id] = true
        expandedGroups = nextGroups
        expandedDevices = nextDevices
    }

    function availableGroups() {
        const found = {}
        const result = []
        for (let i = 0; i < groupNames.length; ++i) {
            const group = groupNames[i]
            if (!found[group]) { found[group] = true; result.push(group) }
        }
        for (let i = 0; i < devices.length; ++i) {
            const group = deviceGroup(devices[i])
            if (!found[group]) { found[group] = true; result.push(group) }
        }
        result.sort(function(a, b) { return a.localeCompare(b, "pt-BR") })
        return result
    }

    function isCameraOpen(deviceId, channelIndex) {
        for (let i = 0; i < mosaicSlots.length; ++i) {
            const tile = mosaicSlots[i]
            if (tile.occupied && tile.deviceId === deviceId && tile.channel === channelIndex
                && tile.status === "Reprodução solicitada")
                return true
        }
        return false
    }

    function deviceHasOpenVideo(deviceId) {
        for (let i = 0; i < mosaicSlots.length; ++i) {
            const slot = mosaicSlots[i]
            if (slot.occupied === true && slot.deviceId === deviceId)
                return true
        }
        return false
    }

    function deviceIconColor(device) {
        if (device.state === "Failed") return "#f28b82"
        if (device.method === "Cloud" && device.state === "Connected") {
            if (device.cloudRoute === "Relay") return "#f3c86a"
            if (device.cloudRoute === "Direct") return "#63a1ff"
        }
        return "#7893b3"
    }

    function buildTree() {
        const result = []
        const search = cameraSearch.text.trim().toLocaleLowerCase()
        const searching = search.length > 0 || favoritesOnly
        const names = availableGroups()
        for (let g = 0; g < names.length; ++g) {
            const group = names[g]
            const groupMatches = search && group.toLocaleLowerCase().indexOf(search) >= 0
            const matchingDevices = []
            for (let i = 0; i < devices.length; ++i) {
                const device = devices[i]
                if (deviceGroup(device) !== group) continue
                const nameMatches = device.name.toLocaleLowerCase().indexOf(search) >= 0
                const matchedChannels = []
                for (let n = 0; n < device.channels.length; ++n) {
                    const camera = device.channels[n]
                    if (favoritesOnly && !isFavorite(device.id, camera.index)) continue
                    if (!search || groupMatches || nameMatches
                        || camera.name.toLocaleLowerCase().indexOf(search) >= 0)
                        matchedChannels.push(camera)
                }
                const display = !searching || (favoritesOnly ? matchedChannels.length > 0 : 
                                groupMatches || nameMatches || matchedChannels.length > 0)
                if (display) matchingDevices.push({device: device, cameras: matchedChannels})
            }
            if (searching && matchingDevices.length === 0) continue
            result.push({kind: "group", group: group, count: matchingDevices.length})
            if (!searching && !groupExpanded(group)) continue
            for (let d = 0; d < matchingDevices.length; ++d) {
                const node = matchingDevices[d]
                result.push({kind: "device", group: group, device: node.device})
                if (!searching && !deviceExpanded(node.device.id)) continue
                for (let k = 0; k < node.cameras.length; ++k) {
                    result.push({kind: "camera", group: group, device: node.device,
                                 camera: node.cameras[k]})
                }
            }
        }
        return result
    }

    Dialog {
        id: createGroupDialog
        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        title: "Novo grupo / local"
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(330, explorer.width + 40)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        onOpened: { groupInput.text = ""; groupInput.forceActiveFocus() }
        onAccepted: {
            const text = groupInput.text.trim()
            if (text.length > 0) explorer.createGroup(text)
        }
        ColumnLayout {
            width: parent.width
            Label { text: "Nome do grupo"; color: explorer.textPrimary }
            TextField {
                id: groupInput
                Layout.fillWidth: true
                maximumLength: 80
                placeholderText: "Ex.: Condomínio, Portaria, Sede"
            }
        }
    }

    Dialog {
        id: moveDialog
        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        property string deviceId: ""
        title: "Mover dispositivo para grupo"
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(340, explorer.width + 50)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        onOpened: targetGroup.currentIndex = 0
        onAccepted: explorer.moveDevice(deviceId,
                            targetGroup.currentIndex === 0 ? "" : targetGroup.currentText)
        ColumnLayout {
            width: parent.width
            Label { text: "Grupo / local"; color: explorer.textPrimary }
            ComboBox {
                id: targetGroup
                Layout.fillWidth: true
                model: [explorer.ungroupedLabel].concat(explorer.groupNames)
            }
            Label {
                text: "Os grupos ainda são mantidos somente nesta execução."
                color: explorer.textSecondary
                font.pixelSize: 10
            }
        }
    }

    Dialog {
        id: deviceInfoDialog
        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        width: Math.min(390, explorer.width + 110)
        x: Math.max(0, (parent.width - width) / 2)
        y: Math.max(0, (parent.height - height) / 2)
        title: "Informações do dispositivo"
        standardButtons: Dialog.Close
        property var info: ({name: "", state: "", method: "",
                              connectionLabel: "", diagnostic: "", channels: []})

        ColumnLayout {
            width: parent.width
            spacing: 9
            Label {
                text: deviceInfoDialog.info.name
                font.pixelSize: 15
                font.bold: true
                color: explorer.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Label {
                text: "Conexão: " + deviceInfoDialog.info.method
                color: explorer.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Label {
                text: "Estado: " + deviceInfoDialog.info.state
                color: explorer.textSecondary
                Layout.fillWidth: true
            }
            Label {
                text: "Localização: " + explorer.deviceGroup(deviceInfoDialog.info)
                color: explorer.textSecondary
                Layout.fillWidth: true
            }
            Label {
                text: "Canais detectados: " + deviceInfoDialog.info.channels.length
                color: explorer.textSecondary
                Layout.fillWidth: true
            }
            Label {
                text: deviceInfoDialog.info.diagnostic
                color: explorer.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        TextField {
            id: cameraSearch
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.topMargin: 8
            Layout.bottomMargin: 7
            Layout.preferredHeight: 32
            placeholderText: "Pesquisar dispositivo ou câmera"
            placeholderTextColor: explorer.textPrimary
            font.pixelSize: 11
            selectByMouse: true
            background: Rectangle {
                radius: 5
                color: "#111722"
                border.color: cameraSearch.activeFocus ? explorer.accent : explorer.borderColor
            }
            color: explorer.textPrimary
        }

        ListView {
            id: tree
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 1
            model: explorer.treeRows
            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                id: nodeRow
                required property var modelData
                property var node: modelData
                property bool isGroup: node.kind === "group"
                property bool isDevice: node.kind === "device"
                property bool isCamera: node.kind === "camera"
                width: tree.width
                height: isGroup ? 31 : isDevice ? 28 : 26

                Rectangle {
                    anchors.fill: parent
                    color: rowMouse.containsMouse ? "#263141" : "transparent"
                    radius: 3
                }
                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    preventStealing: true
                    drag.threshold: 8
                    drag.target: (nodeRow.isDevice || nodeRow.isCamera)
                                 && node.device.state === "Connected"
                                 ? treeDragGhost : null
                    onReleased: {
                        if (rowMouse.drag.active)
                            treeDragGhost.Drag.drop()
                        treeDragGhost.x = 0
                        treeDragGhost.y = 0
                    }
                    onCanceled: {
                        treeDragGhost.Drag.cancel()
                        treeDragGhost.x = 0
                        treeDragGhost.y = 0
                    }
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            if (nodeRow.isGroup) groupMenu.popup()
                            else if (nodeRow.isDevice) deviceMenu.popup()
                            else if (nodeRow.isCamera) cameraMenu.popup()
                            return
                        }
                        if (nodeRow.isGroup)
                            explorer.setGroupExpanded(node.group, !explorer.groupExpanded(node.group))
                        else if (nodeRow.isDevice)
                            explorer.setDeviceExpanded(node.device.id, !explorer.deviceExpanded(node.device.id))
                        else if (nodeRow.isCamera && node.device.state === "Connected")
                            explorer.openCamera(node.device.id, node.camera.index, true)
                    }
                }

                // Drag a lightweight proxy instead of relocating a virtualized
                // ListView delegate. The target DropArea reads the logical IDs
                // from this item; no passwords, hosts or NetSDK handles travel.
                Rectangle {
                    id: treeDragGhost
                    x: 0
                    y: 0
                    width: Math.min(160, Math.max(65, nodeRow.width - 12))
                    height: 27
                    z: 10
                    radius: 4
                    visible: rowMouse.drag.active
                    color: "#2c4972"
                    border.color: explorer.accent
                    opacity: 0.92

                    property string payloadKind: nodeRow.isDevice ? "recorder"
                                                  : nodeRow.isCamera ? "camera" : ""
                    property string payloadDeviceId: nodeRow.isGroup ? "" : node.device.id
                    property int payloadChannel: nodeRow.isCamera ? node.camera.index : -1

                    Drag.active: rowMouse.drag.active
                    Drag.source: treeDragGhost
                    Drag.keys: nodeRow.isDevice ? ["sentrium/recorder"]
                               : nodeRow.isCamera ? ["sentrium/camera"] : []
                    Drag.hotSpot.x: 14
                    Drag.hotSpot.y: 13
                    Drag.supportedActions: Qt.CopyAction
                    Drag.proposedAction: Qt.CopyAction

                    Label {
                        anchors.centerIn: parent
                        width: parent.width - 10
                        text: nodeRow.isGroup ? "" : nodeRow.isDevice
                              ? "▣  " + node.device.name
                              : "▶  " + node.camera.name
                        color: "white"
                        elide: Text.ElideRight
                        font.pixelSize: 11
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: nodeRow.isGroup ? 3 : nodeRow.isDevice ? 17 : 35
                    anchors.rightMargin: 4
                    spacing: 5
                    ToolButton {
                        id: treeExpandArrow
                        visible: !nodeRow.isCamera
                        implicitWidth: visible ? 16 : 0
                        implicitHeight: 23
                        padding: 0
                        background: Item {}
                        contentItem: Label {
                            text: treeExpandArrow.text
                            color: explorer.textSecondary
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        text: nodeRow.isGroup
                            ? (explorer.groupExpanded(node.group) || cameraSearch.text.length
                                || explorer.favoritesOnly ? "▾" : "▸")
                            : (nodeRow.isDevice
                                ? (explorer.deviceExpanded(node.device.id) || cameraSearch.text.length
                                   || explorer.favoritesOnly ? "▾" : "▸") : "")
                        font.pixelSize: 12
                        onClicked: {
                            if (nodeRow.isGroup)
                                explorer.setGroupExpanded(node.group, !explorer.groupExpanded(node.group))
                            else
                                explorer.setDeviceExpanded(node.device.id,
                                                            !explorer.deviceExpanded(node.device.id))
                        }
                    }

                    Rectangle {
                        implicitWidth: 11
                        implicitHeight: 11
                        radius: nodeRow.isCamera ? 5 : 2
                        color: "transparent"
                        border.color: nodeRow.isGroup ? "#77a1d6"
                            : nodeRow.isDevice ? explorer.deviceIconColor(node.device)
                            : explorer.isCameraOpen(node.device.id, node.camera.index)
                              ? "#58c688" : "#71859c"
                        border.width: 1
                        Rectangle {
                            visible: nodeRow.isCamera &&
                                     explorer.isCameraOpen(node.device.id, node.camera.index)
                            anchors.centerIn: parent
                            width: 5
                            height: 5
                            radius: 3
                            color: "#58c688"
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: nodeRow.isGroup ? node.group
                             : nodeRow.isDevice ? node.device.name : node.camera.name
                        color: nodeRow.isCamera ? explorer.textSecondary : explorer.textPrimary
                        font.pixelSize: nodeRow.isGroup ? 12 : nodeRow.isDevice ? 11 : 11
                        font.bold: nodeRow.isGroup
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }


                }

                Menu {

                    // A separate native popup stays above the embedded NetSDK video HWND.

                    popupType: Popup.Window

                    palette.text: explorer.textPrimary

                    palette.buttonText: explorer.textPrimary

                    palette.highlight: "#2e4d78"

                    background: Rectangle {

                        color: "#161f2b"

                        border.color: "#41516a"

                        radius: 5

                    }
                    id: groupMenu
                    MenuItem {
                        text: "Criar grupo / local..."
                        onTriggered: createGroupDialog.open()
                    }
                    MenuItem {
                        text: "Expandir todos"
                        onTriggered: explorer.expandAll()
                    }
                    MenuItem {
                        text: "Recolher todos"
                        onTriggered: explorer.collapseAll()
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: explorer.favoritesOnly ? "Mostrar todas as câmeras" : "Somente favoritas"
                        onTriggered: explorer.favoritesOnly = !explorer.favoritesOnly
                    }
                }

                Menu {

                    // A separate native popup stays above the embedded NetSDK video HWND.

                    popupType: Popup.Window

                    palette.text: explorer.textPrimary

                    palette.buttonText: explorer.textPrimary

                    palette.highlight: "#2e4d78"

                    background: Rectangle {

                        color: "#161f2b"

                        border.color: "#41516a"

                        radius: 5

                    }
                    id: deviceMenu
                    MenuItem {
                        text: "Conectar / Desconectar"
                        enabled: nodeRow.isDevice
                            && (node.device.state === "Disconnected"
                                || node.device.state === "Failed"
                                || node.device.state === "Connected"
                                || (node.device.method === "AutoRegistration"
                                    && node.device.state === "Connecting"))
                        onTriggered: {
                            if (node.device.state === "Connected"
                                || (node.device.method === "AutoRegistration"
                                    && node.device.state === "Connecting"))
                                explorer.disconnectDevice(node.device.id)
                            else
                                explorer.connectDevice(node.device.id)
                        }
                    }

                    Menu {

                        // A separate native popup stays above the embedded NetSDK video HWND.

                        popupType: Popup.Window

                        palette.text: explorer.textPrimary

                        palette.buttonText: explorer.textPrimary

                        palette.highlight: "#2e4d78"

                        background: Rectangle {

                            color: "#161f2b"

                            border.color: "#41516a"

                            radius: 5

                        }
                        title: "Canais"
                        MenuItem {
                            text: "Abrir todas as câmeras em stream Extra"
                            enabled: nodeRow.isDevice && node.device.state === "Connected"
                                && node.device.channels.length > 0
                            onTriggered: explorer.openRecorder(node.device, true)
                        }
                        MenuItem {
                            text: "Abrir todas as câmeras em stream Principal"
                            enabled: nodeRow.isDevice && node.device.state === "Connected"
                                && node.device.channels.length > 0
                            onTriggered: explorer.openRecorder(node.device, false)
                        }
                        MenuSeparator {}
                        MenuItem {
                            text: "Fechar canais deste dispositivo"
                            enabled: nodeRow.isDevice && explorer.deviceHasOpenVideo(node.device.id)
                            onTriggered: explorer.closeDeviceChannels(node.device.id)
                        }
                    }

                    Menu {

                        // A separate native popup stays above the embedded NetSDK video HWND.

                        popupType: Popup.Window

                        palette.text: explorer.textPrimary

                        palette.buttonText: explorer.textPrimary

                        palette.highlight: "#2e4d78"

                        background: Rectangle {

                            color: "#161f2b"

                            border.color: "#41516a"

                            radius: 5

                        }
                        title: "Stream padrão"
                        MenuItem {
                            text: "Principal (a implementar)"
                            enabled: false
                        }
                        MenuItem {
                            text: "Extra (a implementar)"
                            enabled: false
                        }
                    }

                    Menu {

                        // A separate native popup stays above the embedded NetSDK video HWND.

                        popupType: Popup.Window

                        palette.text: explorer.textPrimary

                        palette.buttonText: explorer.textPrimary

                        palette.highlight: "#2e4d78"

                        background: Rectangle {

                            color: "#161f2b"

                            border.color: "#41516a"

                            radius: 5

                        }
                        title: "Áudio"
                        MenuItem {
                            text: "Áudio bidirecional (não implementado)"
                            enabled: false
                        }
                    }

                    Menu {

                        // A separate native popup stays above the embedded NetSDK video HWND.

                        popupType: Popup.Window

                        palette.text: explorer.textPrimary

                        palette.buttonText: explorer.textPrimary

                        palette.highlight: "#2e4d78"

                        background: Rectangle {

                            color: "#161f2b"

                            border.color: "#41516a"

                            radius: 5

                        }
                        title: "Dispositivo"
                        MenuItem {
                            text: "Informações"
                            enabled: nodeRow.isDevice
                            onTriggered: {
                                deviceInfoDialog.info = node.device
                                deviceInfoDialog.open()
                            }
                        }
                        MenuItem {
                            text: "Configuração (não implementada)"
                            enabled: false
                        }
                    }

                    MenuSeparator {}
                    MenuItem {
                        text: "Mover para grupo..."
                        enabled: nodeRow.isDevice
                        onTriggered: {
                            moveDialog.deviceId = node.device.id
                            moveDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Remover dispositivo"
                        enabled: nodeRow.isDevice && node.device.state !== "Disconnecting"
                        onTriggered: explorer.removeDevice(node.device.id)
                    }
                }

                Menu {

                    // A separate native popup stays above the embedded NetSDK video HWND.

                    popupType: Popup.Window

                    palette.text: explorer.textPrimary

                    palette.buttonText: explorer.textPrimary

                    palette.highlight: "#2e4d78"

                    background: Rectangle {

                        color: "#161f2b"

                        border.color: "#41516a"

                        radius: 5

                    }
                    id: cameraMenu
                    MenuItem {
                        text: "Abrir no mosaico — Extra"
                        enabled: nodeRow.isCamera && node.device.state === "Connected"
                        onTriggered: explorer.openCamera(node.device.id, node.camera.index, true)
                    }
                    MenuItem {
                        text: "Abrir no mosaico — Principal"
                        enabled: nodeRow.isCamera && node.device.state === "Connected"
                        onTriggered: explorer.openCamera(node.device.id, node.camera.index, false)
                    }
                    MenuSeparator {}
                    Menu {
                        // A separate native popup stays above the embedded NetSDK video HWND.
                        popupType: Popup.Window
                        palette.text: explorer.textPrimary
                        palette.buttonText: explorer.textPrimary
                        palette.highlight: "#2e4d78"
                        background: Rectangle {
                            color: "#161f2b"
                            border.color: "#41516a"
                            radius: 5
                        }
                        title: "Inteligência de vídeo"
                        MenuItem {
                            text: nodeRow.isCamera &&
                                  explorer.isLiveEventsSelected(node.device.id, node.camera.index)
                                  ? "Retirar canal do painel Eventos ao vivo"
                                  : "Incluir canal no painel Eventos ao vivo"
                            enabled: nodeRow.isCamera
                            onTriggered: explorer.toggleLiveEvents(
                                node.device.id, node.camera.index)
                        }
                        MenuItem {
                            text: "IA do servidor (não integrada)"
                            enabled: false
                        }
                    }
                    MenuItem {
                        text: nodeRow.isCamera &&
                              explorer.isFavorite(node.device.id, node.camera.index)
                              ? "Remover favorito" : "Adicionar aos favoritos"
                        enabled: nodeRow.isCamera
                        onTriggered: explorer.toggleFavorite(node.device.id, node.camera.index)
                    }
                }
            }
        }


    }
}
