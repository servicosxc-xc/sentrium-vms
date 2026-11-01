# SENTRIUM VMS — mudanças UI-005 a UI-007 (menus e indicadores)

Referência: XC Video IA Foundation 007 r25, `Views/Controls/VideoTileControl.xaml`, `ViewModels/MosaicTileViewModel.cs`, `VisualizationTabViewModel.cs`, `DeviceTreeNodeViewModel.cs` e capturas fornecidas pelo operador em 31/10/2026.

**Status:** código versionado em `feature/r25-ui-parity` (PR #5 **draft**). Não compilado no Windows nem aprovado visualmente; não criar release/merge sem confirmação. A implementação respeita o backend real existente, sem simular IA, eventos, áudio ou gravação.

## Alteração 5: menu de câmera no mosaico

Um componente `apps/sentrium-client/CameraContextMenu.qml` compartilhado por todos os tiles inclui:

- Fechar canal: `DeviceController.clearLiveSlot(slot)`.
- Fechar todos: `DeviceController.clearAllLive()`.
- Visualização: selecionar tile e maximizar/restaurar sem derrubar outros streams.
- Stream: trocar Principal/Extra reabrindo o stream correspondente no mesmo slot; Automático indisponível.
- Áudio: submenu exibido com itens inativos até existir implementação.
- Gravação / captura: itens inativos (não fingir snapshot, vídeo ou replay).
- Inteligência de vídeo: ações de backend indisponíveis; seleção local do canal para o painel Eventos ao vivo descrita abaixo.
- Diagnóstico: nome, estado e stream **reportados pelo modelo**, não métricas inventadas.
- Adicionar/remover favoritos: mesmo estado temporário que a árvore de dispositivos.
- Configuração do canal: indisponível enquanto não houver editor funcional.

Abertura pelo botão direito no cabeçalho do tile, e tentativa de captura de clique direito sobre vídeo nativo. **Na janela Qt, `WindowContainer` pode capturar eventos nativos**, então o comportamento sobre a imagem deve ser verificado no Windows. Não duplicar 64 menus.

## Alteração 6: menu do gravador

Em `DeviceExplorer.qml`:

- Conectar/Desconectar, conforme conexão real.
- Canais → abrir todos Extra / Principal, ou fechar somente streams pertencentes ao gravador. Para isso foi adicionado `DeviceController.closeDeviceChannels(id)`, chamando `clearLiveForDevice`, sem logout.
- Stream padrão → escolha futura não implementada (disabled).
- Áudio → ainda indisponível (disabled).
- Dispositivo → informações reais (grupo, método, estado, contagem de canais e diagnóstico). Configurações não implementadas (disabled).
- Mover para grupo e remover dispositivo mantidos.

`openRecorder(deviceData, extra)` agora respeita Extra/Principal no preenchimento do mosaico.

## Alteração 7: indicadores no topo de cada câmera

A r25 mostra dois conceitos independentes:

1. **IA**: badge roxo/marrom conforme motor Person/Vehicle/Motion, condicionado a uma regra/engine realmente ativa no Server. A Foundation 005 ainda não possui servidor de IA verificado. `DeviceController.mosaicSlots()` expõe explicitamente `aiState = "Unavailable"` e `aiMode = ""`; o badge **não é mostrado ativo** sem sinal real de integração. A UI tem ponto de binding para estado `Active` futuro, sem inferir a partir de RealPlayEx.
2. **EVENTOS**: badge azul quando o canal foi **selecionado pelo operador** para o painel Eventos ao vivo desta Visualização (sem ativar/parar IA no Server). Esse estado está em `mosaicWorkspace.liveEventSelections` (temporário em memória), compartilhado entre menu do tile e menu da câmera na árvore de dispositivos. Seleção local **não cria nem garante eventos**: o feed na Foundation 005 não está integrado. Tooltip e menu explicam isso.

O cabeçalho foi reorganizado em `RowLayout` compacto: nome da câmera/gravador, IA quando confirmada, EVENTOS quando selecionado e Extra/Principal. Para tiles estreitos, EVENTOS é abreviado como `EV`. O status de vídeo continua como era; um handle NetSDK isolado não prova frames decodificados.

## Limitações e validações futuras

- **Qt/Windows build e comportamento em hardware: NOT RUN.** Verificação estática de estrutura não aprova este lote.
- Áudio, gravação, captura, configurações detalhadas, IA Server e feed de Eventos ao vivo precisam de backend próprio.
- Seleção Eventos ao vivo e favoritos não persistem, e o estado ainda é global à implementação atual de mosaico; múltiplas abas de Visualização **não** possuem sessões de vídeo separadas (contrato r25 futuro).
- Evitar confundir marcador de Eventos ao vivo com status IA ou com stream ativo.
- Não alterar F005 base, main, F004, Cloud/P2P ou autenticação NetSDK.
- Validar menu sobre imagem/cabeçalho, eventos escolhidos via dois menus, troca de stream, fechar canais individualmente/por gravador, rotulagem responsiva e ferramenta IA inativa após carregar novo dispositivo real.

**Próximo passo:** coletar novas solicitações do usuário; **não gerar nova versão** até aprovação explícita.
