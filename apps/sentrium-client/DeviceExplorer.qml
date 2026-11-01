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
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    property color borderColor: "#2b3442"
    property color panelAltColor: "#1d2430"
    property var favorites: ({})
    property var expandedGroups: ({})
    property var expandedDevices: ({})
    property bool favoritesOnly: false
    property string ungroupedLabel: "Sem grupo"
    readonly property var treeRows: buildTree()

    signal openCamera(string deviceId, int channelIndex, bool extra)
    signal openRecorder(var deviceData)
    signal connectDevice(string deviceId)
    signal disconnectDevice(string deviceId)
    signal removeDevice(string deviceId)
    signal createGroup(string groupName)
    signal moveDevice(string deviceId, string groupName)

    function deviceGroup(device) {
        return device.group && device.group.length ? device.group : ungroupedLabel
    }

    function cameraKey(deviceId, channelIndex) {
        return deviceId + ":" + channelIndex
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

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            TextField {
                id: cameraSearch
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                placeholderText: "Pesquisar sites, dispositivos, canais"
                font.pixelSize: 11
                selectByMouse: true
            }
            ToolButton {
                text: "☆"
                checkable: true
                checked: explorer.favoritesOnly
                onClicked: explorer.favoritesOnly = checked
                ToolTip.visible: hovered
                ToolTip.text: checked ? "Mostrar todos" : "Somente favoritos"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 3
            Label {
                Layout.fillWidth: true
                text: "SITES · DISPOSITIVOS · CANAIS"
                color: explorer.textSecondary
                font.pixelSize: 9
                font.weight: Font.DemiBold
            }
            ToolButton {
                text: "+ Grupo"
                font.pixelSize: 10
                onClicked: createGroupDialog.open()
                ToolTip.visible: hovered
                ToolTip.text: "Criar um grupo/local"
            }
            ToolButton {
                text: "▾"
                implicitWidth: 24
                font.pixelSize: 12
                onClicked: explorer.expandAll()
                ToolTip.visible: hovered
                ToolTip.text: "Expandir tudo"
            }
            ToolButton {
                text: "▸"
                implicitWidth: 24
                font.pixelSize: 12
                onClicked: explorer.collapseAll()
                ToolTip.visible: hovered
                ToolTip.text: "Recolher tudo"
            }
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
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            if (nodeRow.isDevice) deviceMenu.popup()
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

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: nodeRow.isGroup ? 3 : nodeRow.isDevice ? 17 : 35
                    anchors.rightMargin: 4
                    spacing: 5
                    ToolButton {
                        visible: !nodeRow.isCamera
                        implicitWidth: visible ? 16 : 0
                        implicitHeight: 23
                        padding: 0
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

                    Label {
                        visible: nodeRow.isGroup
                        text: String(node.count)
                        color: explorer.textSecondary
                        font.pixelSize: 10
                    }

                    ToolButton {
                        visible: nodeRow.isCamera
                        implicitWidth: visible ? 24 : 0
                        implicitHeight: 25
                        padding: 0
                        text: nodeRow.isCamera
                            ? (explorer.isFavorite(node.device.id, node.camera.index) ? "★" : "☆")
                            : ""
                        font.pixelSize: 13
                        onClicked: explorer.toggleFavorite(node.device.id, node.camera.index)
                    }

                    ToolButton {
                        visible: nodeRow.isDevice || nodeRow.isCamera
                        implicitWidth: visible ? 23 : 0
                        implicitHeight: 25
                        text: "⋯"
                        padding: 0
                        font.pixelSize: 15
                        onClicked: {
                            if (nodeRow.isDevice) deviceMenu.popup()
                            else cameraMenu.popup()
                        }
                    }
                }

                Menu {
                    id: deviceMenu
                    MenuItem {
                        text: "Abrir canais (Extra)"
                        enabled: nodeRow.isDevice && node.device.state === "Connected"
                            && node.device.channels.length > 0
                        onTriggered: explorer.openRecorder(node.device)
                    }
                    MenuItem {
                        text: "Mover para grupo..."
                        enabled: nodeRow.isDevice
                        onTriggered: {
                            moveDialog.deviceId = node.device.id
                            moveDialog.open()
                        }
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: "Conectar"
                        enabled: nodeRow.isDevice
                            && (node.device.state === "Disconnected" || node.device.state === "Failed")
                        onTriggered: explorer.connectDevice(node.device.id)
                    }
                    MenuItem {
                        text: "Desconectar"
                        enabled: nodeRow.isDevice
                            && (node.device.state === "Connected"
                                || (node.device.method === "AutoRegistration"
                                    && node.device.state === "Connecting"))
                        onTriggered: explorer.disconnectDevice(node.device.id)
                    }
                    MenuItem {
                        text: "Remover dispositivo"
                        enabled: nodeRow.isDevice && node.device.state !== "Disconnecting"
                        onTriggered: explorer.removeDevice(node.device.id)
                    }
                }

                Menu {
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

        Label {
            Layout.fillWidth: true
            visible: explorer.treeRows.length === 0
            text: explorer.devices.length === 0 && explorer.groupNames.length === 0
                ? "Nenhum dispositivo cadastrado"
                : "Nenhum resultado para este filtro"
            color: explorer.textSecondary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            text: "Favoritos e organização sem persistência nesta etapa."
            color: explorer.textSecondary
            font.pixelSize: 10
        }
    }
}
