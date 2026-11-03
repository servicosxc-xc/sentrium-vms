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

## Melhoria 4 — reconexão Registro Automático sem reconfigurar NVR

Usuário encontrou falha de operação real: depois da primeira autenticação, NVR não volta sozinho após perda de conexão, a menos que grave novamente o cadastro de AutoRegistro no próprio equipamento. Comparação com SIM Next sugere lacuna de watchdog no SENTRIUM. **Causa concreta** localizada nos callbacks NetSDK de disconnect/reconnect vazios e no bloqueio por sessão existente/stale; além disso não existia retry ativo do app para AutoRegistro.

Mudanças implementadas em `feature/r25-ui-parity`: callback NetSDK propagado ao Worker com Qt QueuedConnection e mutex; retry QTimer a cada 15s para dispositivo já cadastrado com endpoint real conhecido; reciclagem controlada do login antigo após queda; UI retorna para Connecting e restaura slots/canais/streams quando login é confirmado; deslogar manualmente cancela a intenção de reconectar; stream 16→64 em worker. A documentação do NetSDK indica que login Server Conn/AutoRegister não possui a mesma reconexão automática nativa que TCP; o retry explícito é necessário. Os detalhes e critérios estão em `docs/foundation-005/F005-AUTOREG-RECOVERY-RC02.md`.

**Ainda NÃO testado no Windows nem validado em NVR.** Não lançar nova versão até autorização específica. RC01 e branch principal preservadas.


## Melhoria 5 — RC02: clique direito e arraste nativo dos tiles; setas sem fundo (03/11/2026)

**Relato real após testar RC02:** Registro Automático *aparentemente* melhorou (ainda não homologado por testes de queda repetidos); o operador rejeitou o botão de três pontos no tile, quer menu com botão direito igual à árvore de dispositivos, verificou preenchimento da seta de expandir e informou que tile→tile continuou sem funcionar.

**Causa estrutural:** `WindowContainer` apresenta um `QWindow` nativo acima dos itens Qt Quick; o handler de drag anterior dependia de `MouseArea.onReleased` no cabeçalho e do Qt Quick receber eventos sobre HWNDs. Botão `⋯` foi alternativa rejeitada pelo usuário. Qt 6.12 documenta limitação de stacking de child windows. Qt Quick Item suporta `mapFromGlobal()` para transformar ponteiro global em coordenadas locais do mosaico.

**Alterações na branch `feature/r25-ui-parity`, sem modificar RC02:**

- `DeviceController.hpp/.cpp`: `QAbstractNativeEventFilter` instalado na GUI thread. Intercepta **somente** WM_RBUTTONUP e WM_LBUTTONDOWN das superfícies `QWindow` associadas aos tiles (inclusive child HWND do SDK via `IsChild`); nunca consome as mensagens e não interfere nos callbacks NetSDK. Qt QueuedConnection envia `videoSurfaceContextRequested(slot, globalX, globalY)` ou `videoSurfaceLeftPressed(slot)` à QML. `isLeftMouseButtonDown()` lê estado real da tecla esquerda por WinAPI e `cursorScreenPos()` obtém coordenada lógica do cursor pelo Qt.
- `Main.qml`: removido completamente o botão `⋯`. Clique direito no cabeçalho abre o menu pelo `MouseArea`; clique direito em vídeo nativo é encaminhado pelo novo filtro de mensagens, posicionando o mesmo `CameraContextMenu` no local do cursor. Nenhuma ação foi simulada.
- `Main.qml`: arrastar vídeo pelo cabeçalho **ou pela imagem** inicia monitor de ponteiro global a cada 16 ms, com limiar de movimento de 8px. Ao soltar em tile válido, `swapLiveSlots()` existente move/troca canais. `videoMosaic.mapFromGlobal()` e `Repeater.itemAt()` dão destino inclusive quando o HWND de vídeo não envia `MouseArea.onReleased` ao QML. Sem QML `Drag.drop` para tile→tile.
- `DeviceExplorer.qml`: `ToolButton` da seta da árvore com `background: Item {}` e ícone exclusivamente texto `▾`/`▸`; sem preenchimento, mantendo clique de expandir/recolher.

**Segurança/limites:** eventos nativos não carregam host, ID de registro, senha ou frames de vídeo; o filtro não toca NetSDK em thread de callback. Permanecem eventuais limitações de recebimento de `WM_RBUTTONUP` em superfícies que usem subclasse customizada do SDK: precisa validar em Windows. A visualização de recepção de eventos e o drag não foram compilados nem testados no Windows depois desse patch. Não afirmar que o bug está definitivamente resolvido até o próximo teste real.

**Estado:** somente branch de desenvolvimento atualizada. `release-candidate/visualizacao-r25-rc02`, RC01, F005 e main preservadas. Não criar RC03 até autorização expressa.
