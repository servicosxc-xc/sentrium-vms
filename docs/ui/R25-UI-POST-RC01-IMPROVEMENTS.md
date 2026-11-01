# SENTRIUM VMS — ajustes após RC01, melhoria 1 (01/11/2026)

**Solicitação do operador:** coletar novas melhorias, trabalhando na branch de desenvolvimento sem gerar nova versão até autorização expressa.

## Melhoria 1 — Cabeçalho e abas

- O usuário enviou duas capturas do RC01 e pediu remover o grande espaço vazio entre a marca/nome do software e a divisória das abas. Em `apps/sentrium-client/Main.qml`, o `ColumnLayout` da marca recebeu `Layout.minimumWidth`, `preferredWidth` e `maximumWidth` fixados em 187px, eliminando o crescimento indevido que afastava a divisão/abas para a direita.
- Removidos da faixa superior os dois atalhos fixos `+ Nova visualização` e `Reprodução`. **Somente Home permanece como aba inicial fixa**; as demais abas (Visualização, Dispositivos, Reprodução, Configurações etc.) aparecem dinamicamente quando o usuário as abre.
- Não foi excluída a funcionalidade de criar uma nova Visualização nem Reprodução: os respectivos atalhos continuam nos cartões do módulo Home, usando `window.openModule` existente.
- O botão `Configurações` do canto direito permanece, pois não foi incluído nas duas remoções solicitadas.
- Não foi alterado nenhum backend NetSDK/registro, mosaico, Cloud/P2P ou regras IA.

## Estado

**Aplicado apenas à branch `feature/r25-ui-parity`; a branch `release-candidate/visualizacao-r25-rc01` NÃO foi modificada.** Sem build/teste visual ou merge. Não criar versão, release ou incorporar à RC antes do usuário enviar todas as melhorias e autorizar explicitamente. A revisão do posicionamento exato será feita na próxima versão aprovada para teste.

## Melhoria 2 — contraste da pesquisa de câmeras

- O operador identificou o texto da pesquisa lateral com cor preta, ilegível no fundo escuro.
- Em `apps/sentrium-client/DeviceExplorer.qml`, `cameraSearch.placeholderTextColor` passa a usar `explorer.textPrimary` (`#eef2f7`), a mesma cor do texto da aba Home ativa. A cor do texto digitado já utilizava essa propriedade.
- Ajuste somente na branch `feature/r25-ui-parity`, sem alterar a versão RC01. **Sem nova build/release até autorização.**


## Melhoria 3 — corrigir menu de tile e arraste tile → tile

**Relato do operador:** na RC01, o menu contextual de cada tile não abre e não é possível arrastar um tile aberto para outro.

**Causa técnica provável:** o mosaico tem `WindowContainer` com `QWindow` nativo para vídeo NetSDK. A documentação da Qt 6.12 registra que o child window pode estar à frente de Qt Quick; os controles/drop sobre a imagem não são garantidos. O menu anterior podia ficar oculto. As causas efetivas ainda precisam de reprodução após o patch.

**Código modificado somente em `feature/r25-ui-parity`:**

- `CameraContextMenu.qml`: `popupType: Popup.Window`, explicitando que o menu deve aparecer em uma janela popup acima dos HWNDs de vídeo.
- `Main.qml`: chama `cameraContextMenu.popup(popupX, popupY)` com coordenadas explícitas em vez de `popup()` sem posição.
- `Main.qml`: cabeçalho do tile com botão `⋯` sempre acessível, e gesto de botão direito por `TapHandler`, separado da área de clique/arraste esquerdo.
- `Main.qml`: o arraste tile → tile resolve a posição final no `MouseArea.onReleased`, via `mapToItem()` e `mosaicRepeater.itemAt()`, chamando `DeviceController.swapLiveSlots()` já existente. Evita depender de Drag/DropArea sobre o vídeo nativo. Destino recebe destaque de borda.
- `Main.qml`: `sentrium/tile` removido das chaves do `DropArea` para evitar drops duplicados; a árvore de câmera/gravador mantém as chaves originais.
- `apps/sentrium-client/CMakeLists.txt`: reaplicada na branch de trabalho a correção do qmlcachegen de `CameraContextMenu.qml` já validada para recompilar a RC01.

**Critérios para próxima build de teste:** menu no botão `⋯` e clique direito sobre cabeçalho, trocar tile 1→4 vazio e tile 1↔4 ocupado, preservar o stream Extra/Principal e demais canais. Gesto por toda a imagem nativa não é garantido em Qt 6.12: o **cabeçalho é o handle de arraste** e o botão `⋯` fornece acesso garantido ao menu.

**Estado:** código salvo, sem compilação Windows pós-patch e sem aprovação visual. **Não modificar RC01 ou main, nem gerar nova versão até autorização do operador.**
