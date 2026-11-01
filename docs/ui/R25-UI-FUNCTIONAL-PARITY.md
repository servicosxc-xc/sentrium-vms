# SENTRIUM VMS — Paridade funcional e visual com XC Video IA Foundation 007 r25

Data de referência: 2026-10-31. **Estado: baseline de engenharia + UI-001 Home inicial codificada na branch de paridade; build e inspeção Windows pendentes.**

## Princípio obrigatório

A edição **XC-VideoIA-v1-Foundation-007-r25** é o *oráculo de comportamento* do produto antigo: preservar organização, ergonomia, comandos, estados e contratos observados, sem portar WPF/.NET linha a linha. Reimplementar no SENTRIUM C++20 + Qt 6, com pipeline de vídeo, thread model, persistência e testes próprios. O nome do produto permanece **SENTRIUM VMS**. O SIM Next é somente referência funcional, sem copiar seus ativos.

**Fontes primárias inspecionadas:** ZIP `XC-VideoIA-v1-Foundation-007-r25(3).zip` (biblioteca do operador), `src/XC.VideoIA.Client/MainWindow.xaml`, `Views/*.xaml`, `ViewModels/*.cs`, `Resources/Theme.xaml`; auditoria prévia `UI-PARIDADE-R25-PARA-CPP.md` e `MAPA-TESTES-R25-PARA-CPP.md`. Não incluir ZIP r25, dados, credenciais, fotos de dispositivos reais nem binários no GitHub.

**Base C++ comparada:** `foundation/005-device-connectivity`, `apps/sentrium-client/Main.qml`, `DeviceController.cpp` e `DeviceWorker.cpp`. Na base comparada, Eventos/Regras IA/Mídia/LPR apresentam placeholders; Configurações é predominantemente scaffolding; Reprodução é placeholder; Visualização/Dispositivos têm protótipos com vídeo e registro real. Build mosaico anterior PASS no Windows do operador, mas **não valida paridade visual, 16 canais simultâneos nem estabilidade**. Registro Automático em diagnóstico; não alterar conectividade neste trabalho sem testes.

## 1. Shell e identidade visual

Fonte: `MainWindow.xaml`, `Theme.xaml`, `MainWindowViewModel.cs`.

- Janela r25: 1440×900, mínimo 1080×680; chrome próprio; cabeçalho compacto de 44px; tema escuro; status de conexão e telemetria visíveis; barra de abas abertas com fechar; botão **Nova visualização**. Adotar equivalente idiomático Qt e DPI correto — não replicar chamadas WPF.
- Home persistente e módulos abertos em abas fecháveis, múltiplas sessões **independentes de Visualização**. Não fazer todas as abas de Visualização apontarem ao mesmo único estado global.
- Padrões uniformes de espaçamento, cabeçalho, tipografia, tabela, toolbar, árvore, painel e menus de contexto. A r25 deve guiar proporções e densidade, não o código XAML literalmente.
- Indicadores são semânticos: conexão P2P direta azul, Relay amarelo, offline vermelho, câmera ociosa neutra, bolinha verde somente enquanto stream aberto. Estado desconhecido não pode ser exibido como Connected.
- Cada módulo terá um componente QML independente e seu view model/adapter em C++; o `Main.qml` monolítico atual deve evoluir para shell, sem mover dados/inferência para QML.

## 2. Módulos e telas a reconstruir

| Módulo | Fonte r25 | Experiência de referência | Estado F005 de entrada |
|---|---|---|---|
| Home | `HomeView.xaml` | Central de operação com atalhos, estado Server e métricas; nunca inventar telemetria | Cartões/indicadores simplificados |
| Visualização | `VisualizationView.xaml`, `VisualizationTabViewModel.cs`, `MosaicSurface.xaml`, `VideoTileControl.xaml` | Explorer esquerdo aprox. 268px, grupos/sites/gravadores/canais, filtro, favoritos, menus contextuais, eventos ao vivo à direita, rodapé, grids e destaques, seleção/troca/arrastar/maximizar tile; múltiplas abas | Mosaico nativo inicial 1/4/9/16, árvore simples, eventos fictícios/ausentes |
| Dispositivos | `DevicesView.xaml`, `DeviceEditorWindow.xaml`, `DevicesTabViewModel.cs` | Tabela densa: nome/grupo, tipo, conexão, porta, canais, status, série, método; Novo/Editar/Excluir/Conectar/Desconectar/todos; grupo, detalhes e diagnóstico Extra/Principal por canal | Cadastro LAN/AutoRegistro e lista simplificada |
| Eventos | `EventsView.xaml`, `EventDetailsWindow.xaml`, `EventMonitorWindow.xaml` | Feed **global** com filtros Pessoa/Veículo/LPR/Movimento/Linha, detalhe/evidências, revisão manual de placas e monitor ao vivo opcional | Placeholder |
| Regras IA | `AiRulesView.xaml`, `AiRuleEditorWindow.xaml` | Árvore por câmera, várias regras/câmera, classes e confiança, ROI poligonal, linha multiponto, sentido, ocorrência vs repetição com anti-duplicação, foto/vídeo e múltiplas ações configuradas na regra | Placeholder |
| LPR | `LprView.xaml`, `LprProfileEditorWindow.xaml` | Master, perfis por câmera, Extra/Principal de análise, detector/OCR, ROI/gate, burst, consenso, thresholds, dedup, testes, leituras, ground truth, diagnósticos e ações por perfil | Placeholder |
| Mídia | `MediaView.xaml`, `MediaPlaybackWindow.xaml` | Galeria/pesquisa paginada de snapshots e evidências; seleção, visualização foto, próxima/anterior, DAV interno; **não confundir com playback de NVR** | Placeholder |
| Configurações | `SettingsView.xaml` | Sistema/energia, DataRoot/retenção, usuários/papéis (planejado na r25), notificações/SMTP (planejado), Integrações e Muralha Paulista com seleção por canal, CodCamera, coordenadas, histórico e modos | Apenas Registro Automático real e abas vazias |
| Reprodução | Separação nova no SENTRIUM | Timeline e reprodução de gravação do gravador separadas de evidências locais; não fingir que existia como módulo completo no r25 | Placeholder independente |

### 2.1 Visualização — contrato prioritário

- Layout r25: grades 1, 4, 9, 16, 25, 36, 49 e 64; destaques 6, 8, 10, 12 e 14. **F005 apenas 1/4/9/16**, portanto 25–64 são requisitos futuros condicionados a benchmark e pipeline.
- Operações: abrir câmera no tile escolhido, abrir gravador com limite de canais, arrastar árvore→tile, reorganizar, limpar, duplo clique maximizar/restaurar sem perder atribuições, múltiplas sessões de visualização, salvar/restaurar layout + atribuições (sem credenciais).
- Stream: Principal, Extra, Automático (tile maximizado promovido a Principal, outros em Extra). Não habilitar “autotuning” por CPU sem especificação e medição.
- Overlay opcional e eventos ao vivo locais ao posto; processamento de regras IA é **global no Server**, mesmo sem câmera em mosaico.
- Video nativo `QWindow/WindowContainer` é solução provisória de apresentação da F005; não fornece frames ao IA/LPR e impõe limitações de composição/overlays. Pipeline futuro precisa textura Qt/decodificação e backpressure, sem reabrir streams desnecessariamente.
- Menu de contexto com ações reais e habilitação contextual. Funções que r25 mostrava mas não executava (ex.: certas operações snapshot/instant playback/áudio) permanecem pendência explícita.

### 2.2 Dispositivos e conectividade — contrato

- Recuperar hierarquia GRUPO → Dispositivo → Canal, edição de grupo digitável/selecionável, início recolhido, ícones por estado, estrela apenas favoritos e nomes por canal.
- Persistência segura de cadastros/última visualização: especificar e testar em DataRoot separado; a F005 armazena alguns dados só em memória.
- Não alterar sem teste login, Registro Automático, P2P/Relay, handles NetSDK, callbacks, reconexão, credenciais e teardown. Não exibir senha/ID completo nem fazer auto-enrollment de dispositivo inesperado.
- Estado **Connected + lista de canais reais** é condição de aceite antes de iniciar vídeo; handle de RealPlayEx **não prova frame renderizado**.

### 2.3 IA, LPR, Eventos e ações — contrato

- Regras IA e perfis LPR processam em background no servidor, independentemente do tab/tile visível; inferência compartilhada por câmera/regras, com filas limitadas e controle de repetição.
- Evento único por ocorrência, filtros globais, uma única janela de detalhe reutilizável; foto geral + recorte da placa e informações auditáveis; revisões de ground truth.
- Ações configuráveis **na regra IA ou no perfil LPR**, não como fluxo global desvinculado: LivePopup/monitor e PTZ preset foram executores na r25; SMTP efetivo não estava concluído, webhook/relé ficam planejados.
- Muralha Paulista não deve transmitir dados em modo Official sem configuração/validação externa; preservar os campos e a semântica do protocolo e privacidade.

## 3. Ordem de implementação — fatias testáveis

1. **UI-001 Shell/Componentes:** tema e dimensões de referência; abas independentes, navegação real, estrutura modular QML/Qt Test. Nunca alterar `main` ou F004 nesta fatia.
2. **UI-002 Visualização:** hierarquia, favoritos, drag/drop, menus, seleção/duplo clique, layouts 1/4/9/16, switching stream, status e lifecycle; teste com vídeos reais. Evoluir 25–64 só após benchmark.
3. **UI-003 Dispositivos:** tabela/editor completo, grupos e canais; preservação de LAN/AutoReg com testes de reconexão.
4. **UI-004 Eventos/Mídia:** API/eventos globais, filtros, janela única, evidências e reprodução DAV.
5. **UI-005 Regras IA:** editor ROI/linha, regras/câmera, ações e engine headless.
6. **UI-006 LPR:** editor de perfil, ROI/gate, leitura, evidências, OCR/consenso e diagnósticos.
7. **UI-007 Configurações/Integrações:** armazenamento, energia, permissões, notificações e Muralha paulista; funções planejadas ficam identificadas.
8. **UI-008 Reprodução NVR:** módulo independente de timeline/gravações, com NetSDK Playback/PlaySDK e ciclo de vida próprio.

## 4. Critérios de aceite obrigatórios

- Nenhum botão com texto de função implementada pode apenas simular sucesso; disabled ou `não implementado` é preferível a falso operacional.
- Sem regressão do listener/Registro Automático; `Connected`, canais reais, Extra/Principal e fechamento de streams devem ser testados com hardware autorizado em Windows.
- Qt/C++ desacopla renderização, NetSDK e IA/LPR; frame/stream não deve bloquear thread UI; evitar duplicação de streams, filas infinitas e vazamentos ao trocar/fechar aba.
- Cada fatia inclui testes de navegação, estado, teardown e regressão derivados da suíte histórica r25 (151 casos já mapeados; **não** afirmar que foram reexecutados em C++).
- Benchmarks explícitos para 1/4/9/16 e depois 25/36/49/64 antes de aprovar desempenho, CPU, GPU, RAM e tráfego. Sem metas inventadas ou aprovação automática.
- Manter branch isolada de paridade e PR em draft até compilação Qt Windows + validação visual/hardware.

## 5. Restrições atuais

O levantamento original foi exclusivamente documental. O checkpoint UI-001 acima introduz somente a nova Home na branch de paridade; os demais módulos ainda não foram portados. Não houve execução Qt/Windows no ambiente do operador nesta entrega. As imagens/capturas da r25 são referência visual adicional útil; onde o XAML e o view model já definem comportamento, esta baseline é suficiente para começar a implementação sem exigir o ZIP novamente.

## 6. Checkpoint UI-001 — Home operacional (31/10/2026)

**Código alterado apenas em `feature/r25-ui-parity`, PR #5 draft:**

- `apps/sentrium-client/HomeDashboard.qml`: Home em módulo QML próprio, inspirada em `src/XC.VideoIA.Client/Views/HomeView.xaml`, com seções de operação, atalhos em ícones compactos e indicadores derivados do DeviceController real.
- `apps/sentrium-client/Main.qml`: substituído apenas o conteúdo interno da Home; navegação e Controller existentes mantidos. Os módulos de Visualização/Dispositivos, NetSDK e mosaico não foram modificados.
- `apps/sentrium-client/CMakeLists.txt`: registra novo componente em `qt_add_qml_module`.

**Invariantes:** cadastro e contagem de dispositivos reais; sem fingir IA/LPR, Cloud, telemetria de Server ou backend de Reprodução; Home não altera sessões nem abre listeners. Estado `Connected` permanece derivado do DeviceController. As referências originais de UI servem de contrato, não há port WPF literal.

**Validação:** revisão estrutural do diff é possível no GitHub; **build Qt 6.12/MSVC, carregamento da QML e inspeção visual ainda NÃO EXECUTADOS no Windows**. PR #5 permanece draft, sem merge em F005 ou main. Antes de declarar UI-001 pronta, rodar o helper da branch em checkout limpo e conferir ausência de QML errors, ícones e cliques nos 8 módulos. Sem fazer testes Cloud não relacionados.

**Próxima fatia:** UI-002 Visualização e UI-003 Dispositivos, com contratos da r25 (árvore e favoritos, seleção dos tiles, menus contextuais, tabela/editor), porém sem alterar NetSDK/connectividade sem testes.
