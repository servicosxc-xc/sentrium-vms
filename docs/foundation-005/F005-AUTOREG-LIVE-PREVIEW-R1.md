# SENTRIUM F005 — Registro Automático → NetSDK RealPlayEx → janela de vídeo real (R1)

**Prioridade atual (30/10/2026):** operador suspendeu a investigação das DLLs Cloud/P2P/T2U, para obter **imagem ao vivo por Registro Automático**, depois evoluir LPR e inteligência de vídeo. O dispositivo real e o NetSDK Server Connection já estão disponíveis no cliente; não pedir novo serial/credencial no chat.

## Implementado na branch

- O `DeviceWorker` conserva as sessões NetSDK autenticadas pela LAN e pelo Registro Automático. A chamada `connectRegistered` usa `EM_LOGIN_SPEC_CAP_SERVER_CONN` com o **ID de registro definido no gravador**, não é o ID Cloud/P2P; a UI permanece exigindo cadastro explícito.
- O `Adapter` agora oferece `startLive(loginSession,zeroBasedChannel,HWND,extra)` e `stopLive(liveSession)`. Usa `CLIENT_RealPlayEx`/`CLIENT_StopRealPlayEx` da **biblioteca NetSDK oficial já fornecida localmente**, com tipo Principal (`DH_RType_Realplay`) ou Extra (`DH_RType_Realplay_1`). Retorna erro de SDK sanitizado quando a abertura falha. **Não carrega SDK Cloud nem reimplementa streaming proprietário.**
- Em `DeviceController`, `openLive` valida se o gravador está `Connected` e se o canal consta dos canais reais enumerados. Cria uma `QWindow` normal com HWND, abre um único preview por vez e coloca requestStartLive numa fila para worker NetSDK. Fechar a janela solicita `StopRealPlayEx`; desconectar gravador, perder Registro Automático ou fechar aplicativo também interrompe a reprodução antes do logout do NetSDK.
- A árvore de canais em Visualização oferece **▶ Extra** e **▶ Principal** para cada canal de um dispositivo conectado. O vídeo é mostrado **em janela nativa separada**, como primeira prova de campo. Não é ainda o mosaico Qt Quick, e o retorno de handle RealPlayEx não é confirmação de primeiro frame. A imagem precisa ser observada na tela do operador.
- O modo Cloud por túnel desconhecido segue bloqueado na UI/C++, e o script temporário de firewall Lab T2U foi removido no teste anterior. Este fluxo não altera Windows Firewall ou roteador.

## O que já era comprovado antes deste código

- F003: autenticação real NetSDK LAN e enumeração de canais, incluindo disconnect/reconnect.
- F005: Registro Automático por NetSDK, com identificação de equipamento cadastrado e retornos de canais quando a sessão foi validada. É um caminho independente do Cloud/P2P.
- F004: `sentrium-media-lab` obteve fluxo H.264/H.265 via `CLIENT_RealPlayEx` em callbacks e decodificou via PlaySDK em cenários LAN; não tinha janela de vídeo, e um NVR de prova tinha somente três câmeras conectadas.

## Gate de build e teste de campo (ainda NÃO EXECUTADO no Windows)

1. Em PowerShell Developer VS2022 **sem elevação**, atualizar a branch com exceção Git efêmera específica do repositório **somente neste comando**, sem `safe.directory=*` nem alteração de ACL: `git -c safe.directory='C:/Projetos/SENTRIUM VMS/sentrium-vms' pull --ff-only origin foundation/005-device-connectivity`. Conferir sucesso; se não houver confiança na origem desta árvore, não adicionar exceções.
2. Garantir `SENTRIUM_QT_ROOT`, `SENTRIUM_INTELBRAS_SDK_ROOT`, `SENTRIUM_PLAYSDK_ROOT` configurados para SDKs externos. No developer shell que já possui MSVC, inicializar/verificar toolchain conforme scripts existentes; build incremental em output existente: `& $env:SENTRIUM_CMAKE --build --preset sentrium-release --target sentrium-client`. A branch necessita compile/test real e verificação de carregamento de plugins Qt/QML; não presumir `PASS`.
3. Executar cliente pelo ambiente Qt da mesma sessão (Qt/bin, NetSDK/bin, PlaySDK/DLL no PATH). Em Configurações > Registro Automático iniciar listener com IP/porta próprios previamente validados; no NVR configurar Registro Automático com mesmo ID e endereço alcançável; cadastrar dispositivo por Registro Automático no SENTRIUM e aguardar `Connected` com canais reais.
4. Em **Visualização**, abrir canal que tenha câmera ativa e pressionar **▶ Extra**. Verificar **imagem realmente em movimento**, sem mosaico ou frame simulado. Se janela abrir preta, testar **▶ Principal** no mesmo canal e conferir SDK e DLLs presentes e qual stream está ativo no NVR.
5. Fechar janela; confirmar que `StopRealPlayEx` permite abrir outro canal e desconectar normalmente. Testar interrupção de Registro Automático sem travamento e retomar o equipamento. Relatar: compile PASS/FAIL, cliente abre PASS/FAIL, AutoReg connected/channel count, Extra vídeo visível/sem imagem, Principal vídeo visível/sem imagem, fechar/reabrir PASS/FAIL, sem enviar IP/serial/senha.

## Critérios de aceite

`LIVE-001` compile/link cliente e QML no Windows — PENDING.
`LIVE-002` Registro Automático recebe NVR real e enumera canais — PENDING no novo build (historicamente comprovado).
`LIVE-003` RealPlayEx abre stream Extra e mostra **frame real visível** — PENDING.
`LIVE-004` stream Principal/frame real — PENDING.
`LIVE-005` fechar, desconectar, reconectar/limpar handles sem falha — PENDING.

**Próximo sprint depois da evidência:** evoluir de HWND nativo para decode PlaySDK com frame callback, filas limitadas, textura Qt Quick, seleção de tiles e multi-stream, o que também fornece frames reutilizáveis ao detector/tracker/OCR LPR e IA. **Nenhuma IA ou LPR funcional foi implementada nesta entrega.** Escolher Extra para mosaico e Principal para LPR/OCR segundo requisito real de resolução e latência. Não prometer aceleração/hardware ou qualidade OCR antes do primeiro frame e da instrumentação.

**Governança:** F005 branch de trabalho, PR #4 em draft. `main` e F004 Human Review intocados. Nenhum arquivo OEM binário ou segredo no GitHub.


## Evidencia de build real no Windows (30/10/2026, usuario)

Após os bloqueios por `safe.directory` (checkout e vcpkg) e variáveis ausentes `SENTRIUM_QT_ROOT`, o operador restaurou Qt + caminhos NetSDK/PlaySDK do CMakeCache e aplicou **por um único processo** duas exceções `safe.directory` estritas via `GIT_CONFIG_COUNT/KEY/VALUE`, com limpeza `finally`. O build incremental `cmake --build --preset sentrium-release --target sentrium-client` repetiu CMake e exibiu `PREFLIGHT PASS` para Qt6.12.0 MSVC, SDKs x64 e vcpkg baseline; `Configuring done`, `Generating done`, jobs de C++ executados e ultima linha `[62/63] Linking CXX executable sentrium-client.exe`, retornando ao prompt do PowerShell. O arquivo de log completo (2318 linhas) **não contém** mensagens de erros C++/LNK ou `ninja: build stopped`. O log **não mostra o código de saída `$LASTEXITCODE` nem a existencia/timestamp do `.exe`**, por isso registrar como `LIVE-001 BUILD: provável sucesso / confirmação binário pendente`, e não como runtime PASS. Avisos `Could NOT find WrapVulkanHeaders` apareceram sem interromper CMake.

**Proximo gate unico:** na mesma Developer PowerShell (Qt/SDK env ainda presentes) executar `$LASTEXITCODE` e `Get-Item out/sentrium-release/apps/sentrium-client/sentrium-client.exe`, validar EXE presente; preparar `PATH` somente desta sessão com Qt/bin, NetSDK/bin, PlaySDK/DLL, iniciar exe com `Start-Process`; verificar janela. Em seguida cadastrar NVR já autorizado pelo menu Registro Automático e abrir vídeo Extra/Principal em canal REAL. Não pedir ID nem senha no chat. Anotar estado de conexão e frame real observado (não apenas handle). A captura da webcam/gravador ou dados reais não deve ir ao GitHub.


## Primeiro runtime Qt REAL no operador — SENTRIUM UI abriu / AutoRegistration pendente (31/10/2026)

Operador confirmou por screenshot do Windows que `sentrium-client.exe` realmente iniciou a interface Qt com Home, Visualização, Configurações e Dispositivos. `Ready` no topo se refere SOMENTE ao bootstrap da interface, não a conexão ao DVR. Em Dispositivos havia 1 item chamado MAFRAN, método `AutoRegistration`, estado `Connecting`, 0 canais e indicador Conectados 0. **Não inferir listener ativo, login NetSDK, nem imagem!** LIVE-001 runtime UI = PASS (aplicação abriu), LIVE-002/003/004/005 = PENDING.

Inspeção do Main.qml mostra `settingsDialog` em Configurações > Configurar servidor de Registro Automático: o listener é **manual** (botão Ativar servidor), bind default 127.0.0.1, porta default 9500; campos status, Eventos recebidos e último evento. Bind localhost só aceita registro local; para equipamento externo precisa IP local acessível e configuração correspondente do NVR. Não alterar Firewall/NAT sem autorização. Próxima ação mínima é verificar modal de Configuração, status de listening, contador de eventos e última mensagem sem compartilhar IP/ID/senha. Se servidor ativo e nenhum evento, checar endereço/porta alcançáveis e ativação do registro remoto; se ID não cadastrado, conferir ID que o equipamento envia vs cadastro no app; se evento recebido mas login falha, analisar diagnóstico NetSDK sanitizado. Para rede externa, não prometer acesso sem caminho TCP/NAT/VPN autorizado. Cloud/T2U permanece adiado.


## Diagnóstico de campo 31/10/2026 — listener ativo, 73 eventos com ID não cadastrado

Screenshot enviada pelo operador em Configurações — Registro Automático: servidor de registro NetSDK ativo no endereço local configurado e porta configurada; botões Ativar desabilitado/Parar habilitado; status `Registration listener active; waiting for configured device registration IDs`; `Eventos recebidos: 73`; última observação `Registro recebido com ID não cadastrado (verifique o ID no gravador)`. Dispositivos: MAFRAN AutoRegistration Connecting, 0 canais, conectados 0. **O NetSDK recebeu callbacks de registro, mas nenhum desses eventos provou ser do MAFRAN**; nenhum login ou RealPlay foi iniciado. Nenhuma senha/ID/Cloud UUID foi divulgada neste registro.

O código `DeviceWorker::onRegistrationReceived` compara `registrationId` recebido com `target.registrationId` por igualdade EXATA e sensível a maiúsculas; só tenta `Adapter::connectRegistered` após match. O MAFRAN possui no UI um hint do final do ID; isso não prova correspondência. Primeiro verificar o campo **ID de Registro Automático** do gravador vs o mesmo ID no cadastro SENTRIUM, não confundir com Cloud UUID/serial, nem com o nome amigável `MAFRAN`. Se forem distintos, no cliente atual não há edição: excluir o cadastro lógico MAFRAN e recriar pelo método Registro Automático com o mesmo ID que o gravador anuncia, usuário e senha localmente (sem compartilhar no chat); apenas então repetir registro do equipamento e observar status/eventos. Se ambos coincidirem, investigar se o SDK está retornando serial físico em vez do ID de registro, formato/case/truncamento/callback e origem de outras solicitações; não presumir correção por trocar a porta nem mudar firewall ou NAT. Não iniciar debug com logging de ID, credenciais ou IP de dispositivos não cadastrados.

Gate para vídeo: MAFRAN `Connected` e >0 canais, depois ▶ Extra/▶ Principal canal com câmera ativa. Cloud/T2U segue suspenso. main/F004 e PR draft intactos.


## F005-MOSAIC-R1 — mudança de objetivo confirmada pelo operador (31/10/2026)

**Evidência visual real apresentada**: SENTRIUM VMS r005 já autenticou via Registro Automático o gravador MAFRAN e listou 8 canais; o canal 03 mostrou IMAGEM real em janela independente via NetSDK `CLIENT_RealPlayEx` (foto enviada). Prova-se frame visível nesse canal; imagem estática não prova FPS, capacidade multi-canal ou durabilidade. O operador REJEITOU o modelo de janela independente e apontou print do XC Video IA r25 com **mosaico integrado 4x4/16 cameras** como referência funcional. O pedido inclui layouts 1,4,9,16, seleção de canal por tile, vários canais visíveis ao mesmo tempo, status por tile, árvore de dispositivos, preservando AutoRegistro. Nunca reaproveitar/redistribuir a foto de equipamento real em repositório.

**Arquitetura implementada no source GitHub (não compilada após alteração)**: DeviceWorker mantém map de `CLIENT_RealPlayEx` handles por slot (ate 16) e para todos os streams antes de logout/disconnect/re-registro/shutdown; DeviceController mantém até 16 superfícies `QWindow` estáveis CppOwnership e QML `WindowContainer` (Qt 6.12) embute os HWND em tiles de grade 1/4/9/16. Botões Extra/Principal na árvore de dispositivos passam a preencher tiles em vez de criar popup; opção Abrir canais preenche até 16 automaticamente; opção Limpar e seleção de posição disponível. Playback fica como módulo diferente, ainda placeholder, sem fingir funcionalidade. Status `Reprodução solicitada` indica somente handle RealPlayEx, não cada frame decodificado. A UI mantém painéis laterais existentes. As QWindows nativas ficam acima da cena Qt Quick, então cabeçalhos/rodapés dos tiles devem ficar fora da área ocupada por `WindowContainer`; isso foi considerado no layout. Fonte documentação de Qt 6.12: https://doc.qt.io/qt-6.12/qml-qtquick-windowcontainer.html .

**Limites**: programa foi modificado apenas no GitHub na branch F005; nenhuma compilação Windows do novo mosaico ocorreu. Impossível aprovar 16 simultâneos só pela tela de referência ou por testes anteriores 4x com gravador de três cameras. QWindowContainer é ponte para exibir vídeo NetSDK em Qt; ele NÃO fornece frame decodificado ao LPR/IA. Depois de validar mosaico real e teardown, próximo sprint PlaySDK/FFmpeg callback ou hardware decode frame pipeline, sincronização de timestamp, filas limitadas e compartilhamento de frames para OCR/IA; priorizar Principal quando placas legíveis exigirem resolução, Extra para 16 mosaicos.

**Build/run**: helper `scripts/windows/sentrium-mosaic-build-run.ps1` cria exceções Git safe.directory TEMPORÁRIAS e estritas para repositório/vcpkg, lê SDK roots já existentes do CMakeCache, compila Release via VS Developer PowerShell sem elevar, checa exit code e acha EXE por busca exata; registra apenas tail 45 linhas e inicia o cliente. Executar somente apos `git -c "safe.directory=C:/Projetos/SENTRIUM VMS/sentrium-vms" pull --ff-only origin foundation/005-device-connectivity`, com SENTRIUM antigo FECHADO. Status de execução do helper: NOT RUN. Nenhuma regra de firewall ou mudança em main.

**Gate do usuário**: abrir app, reativar listener/config de AutoRegistro, reconectar MAFRAN localmente, abrir canais 03 em Extra e Principal pelo mosaico, testar 1 e 4 tiles com câmera real e depois 9/16 conforme disponibilidade (MAFRAN possui oito canais, não significa oito câmeras físicas ativas); fechar/limpar/reconectar/selecionar slot; fornecer prints sem credenciais. Confirmar UI QML sem erros, renderização in-tile e desempenho antes de LPR/IA.
