# SENTRIUM VMS — UI-008: arrastar câmera/gravador para o mosaico

**Solicitação de 31/10/2026:** permitir arrastar cada câmera para uma posição específica e arrastar um gravador para abrir todas as câmeras dele em **stream Extra**. Como complemento ergonômico, permitir reorganização de câmeras já abertas pelo cabeçalho.

**Branch:** `feature/r25-ui-parity` / PR #5 **draft**. **Sem aprovação do operador, release, merge ou compilação Windows.**

## Implementado

- `DeviceExplorer.qml`: linhas conectadas de gravador ou câmera têm `MouseArea.drag` que movimenta um proxy leve `treeDragGhost`, sem deslocar o delegate virtualizado da `ListView`. Chaves QML internas `sentrium/camera` e `sentrium/recorder` identificam o tipo e expõem ao destino somente `deviceId` e índice do canal. Sem nomes de usuário/senhas, endereços ou handles NetSDK no payload. Não inicia arraste para gravador desconectado.
- `Main.qml`:
  - `DropArea` em cada tile, vazio ou ocupado, com borda destacada ao receber arraste;
  - arrastar **câmera da árvore → tile** abre exclusivamente aquele canal no tile escolhido em Extra. Só o stream de destino é substituído, não fecha os demais;
  - arrastar **gravador da árvore → tile** executa `mosaicWorkspace.dropRecorder()`, a partir da posição escolhida, colocando **todos os canais detectados até 64** em Extra. Expande automaticamente para a menor grade que comporte o trecho contíguo, preservando streams nos tiles não afetados. Caso o trecho ultrapasse a posição 64, continua circularmente do primeiro tile. Não limpa o mosaico inteiro;
  - arrastar **câmera já aberta pelo cabeçalho → outro tile** troca as posições; se o destino estiver vazio, move e libera a posição original. A operação conserva o tipo de stream da câmera já aberta.
- `DeviceController.hpp/.cpp`: `Q_INVOKABLE swapLiveSlots(sourceSlot, targetSlot)` valida limites, presença e estado dos canais; reabre fluxos nas **superfícies nativas dos slots de destino**, sem trocar `QWindow` de lugar ou alterar propriedade do NetSDK. Worker existente continua responsável pelo stop/start.
- Menu contextual, clique e duplo clique do cabeçalho, seleção de tile e layouts existentes foram preservados. O handler de drag usa limiar de movimento para diferenciar clique e arraste.
- Baseado na API de `Drag.keys`, `Drag.active`, `MouseArea.drag.target`, `DropArea.keys` e `DragEvent.source` da **Qt Quick 6.12**.

## Critérios de aceite na próxima build de validação

1. Abrir gravador real com canais via NetSDK; arrastar Canal 01 da árvore para tile 05 (vazio), sem interromper outros.
2. Arrastar Canal 02 para um tile ocupado: substitui **somente o destino**.
3. Arrastar um gravador com 8 canais para tile 03: organiza sequencialmente em Extra, aumentando grade conforme necessário, sem limpar streams fora da faixa afetada.
4. Testar movendo um tile aberto pelo **cabeçalho** a um vazio e a outro ocupado (troca de posição, stream preservado).
5. Testar arraste não realizado/cancelado e dispositivos desconectados: não abrir streams.
6. Testar pelo menos duas disposições (4 e 16), destaque e grade >16 com múltiplas câmeras, sem declarar desempenho 64 canais aprovado só por abrir a interface.
7. **Qt nativo:** `WindowContainer` pode consumir eventos de mouse e drag/drop diretamente sobre a imagem. O **cabeçalho dos tiles** permanece a área de arraste garantida pelo design; a funcionalidade sobre o conteúdo de vídeo precisa de validação no Windows.
8. Sem simulação de vídeo, sem abrir canais falsos e sem mexer em AutoRegistro, autenticação ou Cloud/P2P.

## Estado real

- Código versionado no PR draft; **checagens estáticas** de presença dos sinais, handlers, dimensões e balanceamento de chaves sem erros.
- **Compilação MSVC/Qt, execução do QML, mouse/drag real, aceitação e ordem de eventos do SDK: NÃO TESTADAS.**
- Nenhuma nova versão foi gerada. Continuar coletando ajustes do usuário e aguardar sua aprovação antes do release.
