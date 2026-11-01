import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Camera explorer inspired by the r25 operator workflow.
// Favorites and expanded state are local to this UI until the DataRoot
// persistence contract has been implemented and reviewed.
Rectangle {
    id: explorer
    color: "transparent"

    property var devices: []
    property color textPrimary: "#eef2f7"
    property color textSecondary: "#99a6b8"
    property color accent: "#4f8cff"
    property color borderColor: "#2b3442"
    property color panelAltColor: "#1d2430"
    property var favorites: ({})
    property var expandedDevices: ({})
    property bool favoritesOnly: false

    signal openCamera(string deviceId, int channelIndex, bool extra)
    signal openRecorder(var deviceData)
    signal connectDevice(string deviceId)
    signal disconnectDevice(string deviceId)
    signal removeDevice(string deviceId)

    function cameraKey(deviceId, channelIndex) {
        return deviceId + ":" + channelIndex
    }

    function isFavorite(deviceId, channelIndex) {
        return favorites[cameraKey(deviceId, channelIndex)] === true
    }

    function toggleFavorite(deviceId, channelIndex) {
        const key = cameraKey(deviceId, channelIndex)
        const updated = Object.assign({}, favorites)
        updated[key] = !isFavorite(deviceId, channelIndex)
        favorites = updated
    }

    function setExpanded(deviceId, value) {
        const updated = Object.assign({}, expandedDevices)
        updated[deviceId] = value
        expandedDevices = updated
    }

    function isExpanded(deviceId) {
        return expandedDevices[deviceId] === true
    }

    function visibleDevice(deviceData) {
        if (deviceData === null || deviceData === undefined)
            return false
        const search = cameraSearch.text.trim().toLocaleLowerCase()
        const nameMatches = !search || deviceData.name.toLocaleLowerCase().indexOf(search) >= 0
        if (nameMatches && !favoritesOnly)
            return true
        for (let i = 0; i < deviceData.channels.length; ++i) {
            const camera = deviceData.channels[i]
            if ((!favoritesOnly || isFavorite(deviceData.id, camera.index))
                && (nameMatches || camera.name.toLocaleLowerCase().indexOf(search) >= 0))
                return true
        }
        return false
    }

    function visibleCamera(deviceData, camera) {
        const search = cameraSearch.text.trim().toLocaleLowerCase()
        const nameMatches = deviceData.name.toLocaleLowerCase().indexOf(search) >= 0
        return (!favoritesOnly || isFavorite(deviceData.id, camera.index))
            && (!search || nameMatches || camera.name.toLocaleLowerCase().indexOf(search) >= 0)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            TextField {
                id: cameraSearch
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                placeholderText: "Pesquisar dispositivo ou câmera"
                font.pixelSize: 11
                selectByMouse: true
            }
            ToolButton {
                text: "★"
                checkable: true
                checked: explorer.favoritesOnly
                onClicked: explorer.favoritesOnly = checked
                ToolTip.visible: hovered
                ToolTip.text: checked ? "Mostrar todas as câmeras" : "Mostrar somente favoritos"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                text: "GRUPOS E DISPOSITIVOS"
                color: explorer.textSecondary
                font.pixelSize: 10
                font.bold: true
            }
            ToolButton {
                text: "Recolher"
                font.pixelSize: 10
                onClicked: explorer.expandedDevices = ({})
            }
            ToolButton {
                text: "Expandir"
                font.pixelSize: 10
                onClicked: {
                    const next = {}
                    for (let i = 0; i < explorer.devices.length; ++i)
                        next[explorer.devices[i].id] = true
                    explorer.expandedDevices = next
                }
            }
        }

        ListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 5
            model: explorer.devices

            delegate: Item {
                id: deviceDelegate
                width: deviceList.width
                property var deviceData: modelData
                property bool matches: explorer.visibleDevice(deviceData)
                property bool expanded: explorer.isExpanded(deviceData.id)
                height: matches ? deviceCard.implicitHeight + 2 : 0
                visible: matches

                Rectangle {
                    id: deviceCard
                    width: parent.width
                    implicitHeight: deviceContent.implicitHeight + 12
                    radius: 5
                    color: explorer.panelAltColor
                    border.color: explorer.borderColor

                    ColumnLayout {
                        id: deviceContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 7
                        spacing: 3

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            ToolButton {
                                implicitWidth: 28
                                implicitHeight: 27
                                text: deviceDelegate.expanded ? "▾" : "▸"
                                onClicked: explorer.setExpanded(deviceDelegate.deviceData.id,
                                                                 !deviceDelegate.expanded)
                                ToolTip.visible: hovered
                                ToolTip.text: deviceDelegate.expanded ? "Recolher canais" : "Expandir canais"
                            }
                            Rectangle {
                                implicitWidth: 7
                                implicitHeight: 7
                                radius: 4
                                color: deviceDelegate.deviceData.state === "Failed"
                                    ? "#ff8d8d" : "#8492a7"
                                // Green is reserved for a truly active video tile,
                                // not merely an authenticated recorder.
                            }
                            Label {
                                Layout.fillWidth: true
                                text: deviceDelegate.deviceData.name
                                color: explorer.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            ToolButton {
                                text: "⋯"
                                implicitWidth: 28
                                implicitHeight: 27
                                onClicked: recorderMenu.popup()
                            }
                            Menu {
                                id: recorderMenu
                                MenuItem {
                                    text: "Expandir / recolher"
                                    onTriggered: explorer.setExpanded(deviceDelegate.deviceData.id,
                                                                      !deviceDelegate.expanded)
                                }
                                MenuItem {
                                    text: "Abrir todos (Extra)"
                                    enabled: deviceDelegate.deviceData.state === "Connected"
                                             && deviceDelegate.deviceData.channels.length > 0
                                    onTriggered: explorer.openRecorder(deviceDelegate.deviceData)
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: "Conectar"
                                    enabled: deviceDelegate.deviceData.state === "Disconnected"
                                             || deviceDelegate.deviceData.state === "Failed"
                                    onTriggered: explorer.connectDevice(deviceDelegate.deviceData.id)
                                }
                                MenuItem {
                                    text: "Desconectar"
                                    enabled: deviceDelegate.deviceData.state === "Connected"
                                             || (deviceDelegate.deviceData.method === "AutoRegistration"
                                                 && deviceDelegate.deviceData.state === "Connecting")
                                    onTriggered: explorer.disconnectDevice(deviceDelegate.deviceData.id)
                                }
                                MenuItem {
                                    text: "Remover dispositivo"
                                    enabled: deviceDelegate.deviceData.state !== "Disconnecting"
                                    onTriggered: explorer.removeDevice(deviceDelegate.deviceData.id)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 33
                            spacing: 6
                            Label {
                                Layout.fillWidth: true
                                text: deviceDelegate.deviceData.state + " · "
                                    + deviceDelegate.deviceData.channels.length + " canais"
                                font.pixelSize: 10
                                color: explorer.textSecondary
                            }
                            ToolButton {
                                text: "Abrir canais"
                                font.pixelSize: 10
                                enabled: deviceDelegate.deviceData.state === "Connected"
                                         && deviceDelegate.deviceData.channels.length > 0
                                onClicked: explorer.openRecorder(deviceDelegate.deviceData)
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.leftMargin: 33
                            visible: deviceDelegate.deviceData.state !== "Connected"
                            text: deviceDelegate.deviceData.diagnostic
                            elide: Text.ElideRight
                            font.pixelSize: 10
                            color: deviceDelegate.deviceData.state === "Failed"
                                ? "#ffb4b4" : explorer.textSecondary
                        }

                        Repeater {
                            model: deviceDelegate.expanded ? deviceDelegate.deviceData.channels : []
                            delegate: Item {
                                id: cameraRow
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: visible ? 31 : 0
                                visible: explorer.visibleCamera(deviceDelegate.deviceData, modelData)
                                property bool isOpened: false

                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 5
                                    Item { Layout.preferredWidth: 17 }
                                    ToolButton {
                                        implicitWidth: 26
                                        implicitHeight: 26
                                        text: explorer.isFavorite(deviceDelegate.deviceData.id,
                                                                  cameraRow.modelData.index)
                                              ? "★" : "☆"
                                        onClicked: explorer.toggleFavorite(deviceDelegate.deviceData.id,
                                                                            cameraRow.modelData.index)
                                        ToolTip.visible: hovered
                                        ToolTip.text: "Favorito nesta sessão"
                                    }
                                    Rectangle {
                                        width: 6
                                        height: 6
                                        radius: 3
                                        color: "#586477"
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        text: cameraRow.modelData.name
                                        elide: Text.ElideRight
                                        color: explorer.textPrimary
                                        font.pixelSize: 11
                                    }
                                    ToolButton {
                                        implicitWidth: 27
                                        implicitHeight: 25
                                        text: "▶"
                                        enabled: deviceDelegate.deviceData.state === "Connected"
                                        onClicked: explorer.openCamera(deviceDelegate.deviceData.id,
                                                                       cameraRow.modelData.index, true)
                                        ToolTip.visible: hovered
                                        ToolTip.text: "Abrir no mosaico (Extra)"
                                    }
                                    ToolButton {
                                        implicitWidth: 27
                                        implicitHeight: 25
                                        text: "⋯"
                                        onClicked: cameraMenu.popup()
                                    }
                                }
                                Menu {
                                    id: cameraMenu
                                    MenuItem {
                                        text: "Abrir — Extra"
                                        enabled: deviceDelegate.deviceData.state === "Connected"
                                        onTriggered: explorer.openCamera(deviceDelegate.deviceData.id,
                                                                         cameraRow.modelData.index, true)
                                    }
                                    MenuItem {
                                        text: "Abrir — Principal"
                                        enabled: deviceDelegate.deviceData.state === "Connected"
                                        onTriggered: explorer.openCamera(deviceDelegate.deviceData.id,
                                                                         cameraRow.modelData.index, false)
                                    }
                                    MenuSeparator {}
                                    MenuItem {
                                        text: explorer.isFavorite(deviceDelegate.deviceData.id,
                                                                  cameraRow.modelData.index)
                                              ? "Remover dos favoritos" : "Adicionar aos favoritos"
                                        onTriggered: explorer.toggleFavorite(deviceDelegate.deviceData.id,
                                                                             cameraRow.modelData.index)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: explorer.favoritesOnly ? "Favoritos somente desta sessão" :
                  "Clique em ▸ para expandir. Selecione um tile no mosaico antes de abrir uma câmera."
            color: explorer.textSecondary
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }
    }
}
