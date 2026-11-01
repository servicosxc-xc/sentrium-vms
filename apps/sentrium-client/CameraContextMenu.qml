import QtQuick
import QtQuick.Controls

// Single shared r25-style live-tile context menu, regardless of mosaic size.
// Unsupported backend actions remain explicitly disabled.
Menu {
    id: menu

    property int targetSlot: -1
    property var slots: []
    property int focusedSlot: -1
    property bool favorite: false

    readonly property var tile: targetSlot >= 0 && targetSlot < slots.length
        ? slots[targetSlot] : ({ occupied: false, deviceId: "", channel: -1,
                                extra: true, channelName: "", status: "" })
    readonly property bool hasCamera: tile.occupied === true
        && tile.deviceId !== "" && tile.channel >= 0
    readonly property bool hasAnyCamera: slots.some(function(slot) { return slot.occupied === true })

    signal closeChannel(int slot)
    signal closeAllChannels()
    signal selectTile(int slot)
    signal toggleMaximize(int slot)
    signal setStream(int slot, bool extra)
    signal toggleFavorite(string deviceId, int channelIndex)

    MenuItem {
        text: "Fechar canal"
        enabled: menu.hasCamera
        onTriggered: menu.closeChannel(menu.targetSlot)
    }
    MenuItem {
        text: "Fechar todos os canais"
        enabled: menu.hasAnyCamera
        onTriggered: menu.closeAllChannels()
    }

    Menu {
        title: "Visualização"
        MenuItem {
            text: "Selecionar este tile"
            onTriggered: menu.selectTile(menu.targetSlot)
        }
        MenuItem {
            text: menu.focusedSlot === menu.targetSlot
                ? "Restaurar mosaico" : "Maximizar este tile"
            onTriggered: menu.toggleMaximize(menu.targetSlot)
        }
    }

    Menu {
        title: "Stream"
        MenuItem {
            text: "Automático (a implementar)"
            enabled: false
        }
        MenuItem {
            text: (menu.hasCamera && !menu.tile.extra ? "✓ " : "") + "Principal"
            enabled: menu.hasCamera
            onTriggered: {
                if (menu.tile.extra)
                    menu.setStream(menu.targetSlot, false)
            }
        }
        MenuItem {
            text: (menu.hasCamera && menu.tile.extra ? "✓ " : "") + "Extra"
            enabled: menu.hasCamera
            onTriggered: {
                if (!menu.tile.extra)
                    menu.setStream(menu.targetSlot, true)
            }
        }
    }

    Menu {
        title: "Áudio"
        MenuItem {
            text: "Ouvir áudio (não implementado)"
            enabled: false
        }
        MenuItem {
            text: "Áudio bidirecional (não implementado)"
            enabled: false
        }
    }

    Menu {
        title: "Gravação / captura"
        MenuItem {
            text: "Capturar foto (não implementado)"
            enabled: false
        }
        MenuItem {
            text: "Gravar vídeo (não implementado)"
            enabled: false
        }
        MenuItem {
            text: "Reprodução instantânea (não implementado)"
            enabled: false
        }
    }

    Menu {
        title: "Inteligência de vídeo"
        MenuItem {
            text: "Ativar IA no Ao Vivo (não implementado)"
            enabled: false
        }
        MenuItem {
            text: "Configurar regra IA (não implementado)"
            enabled: false
        }
        MenuItem {
            text: "Overlay de detecções (não implementado)"
            enabled: false
        }
    }

    Menu {
        title: "Diagnóstico"
        MenuItem {
            text: menu.hasCamera ? "Câmera: " + menu.tile.channelName : "Sem câmera neste tile"
            enabled: false
        }
        MenuItem {
            text: menu.hasCamera ? "Estado: " + menu.tile.status : "Canal não selecionado"
            enabled: false
        }
        MenuItem {
            text: menu.hasCamera ? "Stream: " + (menu.tile.extra ? "Extra" : "Principal") : "Stream indisponível"
            enabled: false
        }
    }

    MenuItem {
        text: menu.favorite ? "Remover dos favoritos" : "Adicionar aos favoritos"
        enabled: menu.hasCamera
        onTriggered: menu.toggleFavorite(menu.tile.deviceId, menu.tile.channel)
    }
    MenuItem {
        text: "Configuração do canal (não implementado)"
        enabled: false
    }
}
