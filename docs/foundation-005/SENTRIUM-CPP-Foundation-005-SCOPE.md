# SENTRIUM-CPP-Foundation-005-SCOPE 0.1 — DEVICE CONNECTIVITY

**Data:** 25/10/2026  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Branch:** `foundation/005-device-connectivity`  
**Autorização humana:** **F005-AUTH-001 — AUTHORIZED FOR EXECUTION**, confirmação do responsável “vamos seguir” à proposta de priorizar interface, Cloud/P2P e Registro Automático.  
**Estado:** **AUTHORIZED / IN IMPLEMENTATION — DIRECT GITHUB DEVELOPMENT; DEBUG BUILD REPORTADO PASS PELO OPERADOR**. O desenvolvimento passa a ser feito diretamente neste chat, sem Codex.

## Objetivo verificável

Abrir o `sentrium-client`, adicionar um gravador real por **LAN, Cloud/P2P ou Registro Automático**, estabelecer sessão apropriada, exibir estado verdadeiro e enumerar seus canais reais, permitindo desconectar/remover sem processos remanescentes. **Não** afirmar funcionamento de modalidade não implementada.

## Base e limites

- Reutilizar UI Qt 6.12, `DeviceController`, `DeviceWorker`, `sentrium-device` e `sentrium-intelbras` já existentes; LAN está operacional.
- Branch iniciada de `715b81912c0ba33508bca3fba284fee023bc0fee` (**F004 ainda não fechada**), sem merge em `main`. Esta dependência deve ser resolvida em revisão humana posterior; criar a branch não aprova F004.
- **F004 fica em pausa para benchmarks de escala**: somente duas câmeras úteis no NVR local. Testes reais de 4/16/32 câmeras são `NOT RUN` neste hardware. Não repetir testes de CPU/GPU/handles como pré-requisito à entrega de conectividade.
- Cloud/P2P: implementação **própria, autônoma e nativa do SENTRIUM**. Os arquivos `SIMNext.P2PSDK.dll`, `Libt2uWrapper.dll` e `libt2u.dll` observados no SIM Next são **referência histórica de comportamento e engenharia reversa**, não dependências obrigatórias nem fonte de binários do produto. Nenhum SDK/protocolo/ABI pode ser inventado.
- Registro Automático: investigar APIs de serviço/listen/registration do NetSDK **conforme headers/documentação do kit presente**, com origem, callback, autenticação, teardown e controle de portas explicitados. Não inventar ABI.
- Dependências proprietárias e dados operacionais ficam fora do Git; não realizar downloads nem instalações automáticos.

## Entregas em sequência

1. **F005.1 — Interface operável**: escolha LAN / Cloud-P2P / Registro Automático com campos específicos, validação, estados e erros reais; preservar LAN. Não mostrar “Connected” sem uma sessão real. Opções sem SDK comprovado devem indicar claramente indisponibilidade e não enviar requisições fictícias.
2. **F005.2 — Cloud/P2P real**: prova do transporte com dispositivo real e login NetSDK válido, enumeração de canais, desconexão e reconexão. Distinguir P2P direto (azul) e Relay (amarelo) somente quando SDK reportar a modalidade; desconhecido fica neutro.
3. **F005.3 — Registro Automático real**: serviço explicitamente configurado e desativado por padrão, associação a DeviceId lógico, entrada/saída do dispositivo, enumeração e desconexão/stop. Listener de rede só com configuração/autorização local explícita; não abrir firewall, NAT ou serviços públicos automaticamente.
4. **F005.4 — Validação funcional concentrada**: uma prova funcional LAN e, quando disponíveis, uma prova Cloud e Registro Automático em equipamento real, incluindo gravadores 16/32 canais acessíveis, com log sanitizado. Mídia/live-view definitivo virá depois.

## Requisitos não negociáveis

- Não persistir usuário/senha/ID de registro sensível sem decisão explícita; minimizar vida útil de segredos na memória e sanitizar logs.
- Não acoplar ciclo de vida do túnel P2P ao RealPlay; sessões liberadas e conexões fechadas deterministicamente.
- Mesma identidade lógica de dispositivo e árvore de canais independentemente do transporte.
- Não introduzir broker/framework genérico sem consumidor, nem registros de câmera inventados, fallback de transporte silencioso ou estados falsos.
- Nenhuma alteração da baseline vcpkg, Qt, tags aprovadas ou `main`; sem alterar a F004 retroativamente.
- Nenhum merge ou `HUMAN REVIEW APPROVED` sem revisão humana explícita.

## Fora do escopo

Live View de produção, render Qt Quick/mosaico, playback, gravação, IA/LPR, banco completo, installer, benchmarking multicâmera de escala sem equipamento real e infraestrutura de serviços de exposição pública.

## Critérios de aceite

- **F005-AC-01** — build Debug/Release e regressão mínima do que foi afetado.
- **F005-AC-02** — UI de três métodos e fluxo LAN preservado.
- **F005-AC-03** — Cloud/P2P usa API e binários reais verificados, ou modalidade é explicitamente `BLOCKED` (não `PASS`).
- **F005-AC-04** — Cloud real enumera canais e retorna estado de conexão correto.
- **F005-AC-05** — P2P direto/Relay somente quando observado; estado desconhecido neutro.
- **F005-AC-06** — Registro Automático usa API real verificada e um dispositivo real; ou `BLOCKED`, sem simulação.
- **F005-AC-07** — credenciais não aparecem em evidências e não são persistidas.
- **F005-AC-08** — disconnect/reconnect/remove limpos e sem vazamento/processo residual no caso validado.
- **F005-AC-09** — árvore representa número e canais reais do equipamento.
- **F005-AC-10** — relatório distingue `PASS`, `NOT RUN`, `BLOCKED` e dependência externa para cada modalidade.

**Resultado máximo do executor:** `FOUNDATION-005 — PASS / READY FOR HUMAN REVIEW` somente se os critérios relevantes estiverem comprovados. Aprovação humana posterior.

## Checkpoint de implementação direta — 25/10/2026

- Interface Qt já permite cadastro LAN e cadastro de dispositivo esperado por número de série para Registro Automático.
- Listener real NetSDK implementado com `CLIENT_ListenServer` / `CLIENT_StopListenServer` e retorno `DH_DVR_SERIAL_RETURN`, `DH_DVR_DISCONNECT` e `NET_DEV_AUTOREGISTER_RETURN`.
- Login ao receber registro por `CLIENT_LoginEx2` com `EM_LOGIN_SPEC_CAP_SERVER_CONN` e ID de registro validado; contrato verificado no header Win64 NetSDK 3.050 e no exemplo oficial VehicleGPS do ZIP original preservado fora do Git.
- Associados os eventos à thread Qt existente, sem executar login dentro do callback nativo; dispositivos inesperados não são admitidos automaticamente; listener desligado por padrão até comando na interface.
- **Implementado no código, ainda não compilado ou testado no Windows.** Portanto não declarar PASS nem fazer merge.
- Cloud/P2P continua não implementado; o legado é uma referência de engenharia reversa e **não será requisito de runtime ou de instalação**. A conexão só será habilitada após existir backend nativo próprio, com bootstrap/servidor/credencial oficialmente autorizados ou implementação independente comprovada. UI não simula conexão.

## Ajustes de interface — observação do operador (25/10/2026)

Após abrir o cliente Debug no Windows, o operador apresentou capturas reais do modal LAN / Cloud / Registro Automático e solicitou três correções:

1. Botão `+ Adicionar` aparece como retângulo branco/sem rótulo no primeiro momento; aplicar `background` e `contentItem` explícitos no `Main.qml` para renderização estável (commit `6e61f1fce3f33c3c38adf09341220d94e5d3e0ab`).
2. Clicar fora de `Adicionar dispositivo` **não pode fechar o diálogo**. `closePolicy: Popup.CloseOnEscape` evita fechamento por clique externo e mantém Escape / Cancelar; validar no cliente Windows após rebuild.
3. Os parâmetros do **servidor** de Registro Automático (bind/IP, porta, ligar/desligar, estado) ficam **temporariamente** no diálogo para permitir testes. **Mover para futuro módulo `Configurações > Registro Automático`**, preservando o cadastro do **dispositivo** (ID de registro, usuário, senha) no fluxo `Adicionar dispositivo`. Migrar com separação do estado do listener e dos gravadores, sem acoplar ativação do servidor ao cadastro, e sem habilitar listener automaticamente após a futura migração.

O ajuste foi feito somente em QML e **precisa de rebuild/inspeção visual no Windows** para confirmação. Não afeta o estado `IN IMPLEMENTATION` nem concede Human Review.

## Correção funcional — identificador do Registro Automático

**O operador confirmou em campo:** no gravador Intelbras, a opção Registro Automático recebe um **ID do dispositivo definido pelo operador**, que deve coincidir com o ID cadastrado no servidor SENTRIUM. **Não é necessário usar o número de série físico/fábrica.**

Esta interpretação foi confrontada com o exemplo oficial `VehicleGPSDlg.cpp` do NetSDK 3.050: o callback `DH_DVR_SERIAL_RETURN` atribui `pParam` à variável local `szDevSerial`, porém compara com `m_strListenDevID`, que representa o ID configurado no dispositivo, antes do login `CLIENT_LoginEx2(... EM_LOGIN_SPEC_CAP_SERVER_CONN ...)`. O SDK tem estrutura de configuração `DHDEV_REGISTER_SERVER.szDeviceID[32]` e variante `CFG_REGISTER_SERVER_INFO.szDeviceID`. A nomenclatura de buffer `szDevSerial` é histórica do fabricante e **não define a semântica de cadastro**.

Portanto, na F005 o modelo próprio utiliza `registration_id` / `registrationId`; o formulário diz **ID de Registro Automático**, e o matching exige igualdade ao ID devolvido no callback. **ID Cloud/P2P permanece outro campo, com outra finalidade**. A validação no equipamento real ainda está pendente.

### Referências históricas recebidas em 25/10/2026

- `XC-VideoIA-v1-Foundation-007-r25(3).zip`: contém `src/XC.VideoIA.Infrastructure.Intelbras/Cloud/IntelbrasCloudTunnelService.cs`, que usa `SIMNext.P2PSDK.VVEyeAdapter` por reflexão e métodos `AddSDKPort`, `P2pIsActive`, `RelayConnection` e `DeleteSDKPort`; esta revisão contém preparação de UI para Registro Automático, mas o serviço `IntelbrasDeviceService.cs` indica explicitamente `NOT_IMPLEMENTED` para `AutoRegister`. Isso **não nega** a operação relatada de uma versão anterior/instalação diferente; apenas delimita o ZIP inspecionado.
- `XC-VideoIA-v2-CPP-AUDIT-001 (2)(2).zip`: addendum Cloud 23/10/2026 documenta o caminho `SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll`, portas e status Direct/Relay, porém é levantamento documental e não contém a runtime da DLL.
- `XC-VideoIA-v2-CPP-AUDIT-001(4).zip`: auditoria original reconhece bloqueio de SDK Cloud para implementação autônoma; comparado ao adendo mais recente, esse ponto é atualização documental, não contradição técnica.
- ZIPs fornecidos no chat **não são adicionados ao repositório**; SDK/Cloud proprietários permanecem fora do Git. `F005-LEGACY-REFERENCE-R1.md` registra apenas a orientação necessária.

**Estado:** código de IDs corrigido e versionado, ainda sem rebuild Windows depois dessa correção; Cloud/P2P ainda `BLOCKED` sem inspeção/prova de DLLs e bootstrap autorizado.

## Decisão vinculante de arquitetura — Cloud próprio (25/10/2026)

O responsável determinou: **"não vamos depender dos arquivos do SIM Next para funcionar o Cloud"**. Essa exigência prevalece sobre qualquer menção anterior de utilizar DLLs extraídas da instalação.

- O caminho `SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll` e os exports `t2u_*` recuperados são **somente evidências históricas**. Sua existência não autoriza `LoadLibrary` de arquivos da instalação, redistribuição, extração de credenciais de bootstrap ou dependência do SIM Next.
- Uma implementação autônoma poderá utilizar apenas componentes/protocolos com origem legítima, contrato comprovado e direito de uso; **não** será exigida a instalação do SIM Next, nem de seus arquivos originais, serviços ou configurações.
- Se não houver material suficiente para realizar bootstrap Cloud com autenticação e negociação de túneis, reportar **BLOCKED: protocolo/bootstrap independente não comprovado**. Não usar credenciais embutidas, endpoints inferidos, DLLs terceiras de origem incerta nem afirmar integração pronta.
- O estado do dispositivo, a rota Cloud medida (`Unknown`, `Direct`, `Relay`) e a sessão NetSDK continuam conceitos distintos. `Direct` (azul) e `Relay` (amarelo) só quando a telemetria real comprovar rota e conexão.
- `LAN` e `AutoRegistration` permanecem na mesma identidade lógica. Um túnel Cloud entrega somente um endpoint de controle para NetSDK; seu lifetime é separado do RealPlay.
- Evidência fornecida pelo operador: texto `SENTRIUM VMS — PROGRESSO CLOUD / P2P` datado de 25/10/2026, originalmente documento de progresso "sem autorização de implementação"; a autorização geral posterior da F005 foi concedida no chat, mas **não elimina os requisitos de bootstrap/ABI/licença**.
- Checkpoint de construção recebido: após atualização `1db29e9..a48c833`, o operador informou `[25/25] Linking CXX executable sentrium-client.exe` em Debug, **PASS para esse commit anterior**. As novas alterações de modelo/transporte nesta branch ainda requerem build Windows; não herdar PASS automaticamente.

## Fonte Cloud consolidada e avanço de código — 25/10/2026

**Fonte factual primária importada integralmente** para a branch de trabalho:
`docs/foundation-005/SENTRIUM-CLOUD-P2P-DESCOBERTAS-CONSOLIDADAS.md`
(blob de origem: `cd2eb3a72b795b3a5f648c19e42da9c9449ab161`; arquivo originalmente presente em `foundation/004-media-lab`, não em F005).

Evidência relevante:
- `AddSDKPort(deviceId, uuid, remotePort, localPort)` e `t2u_add_port_v3` são comportamento legado observado, não contrato ABI nativo do novo projeto.
- `remotePort=37000` é observação de **um** equipamento, não default geral; o retorno bem-sucedido é porta TCP **local dinâmica**, usada pelo NetSDK em `127.0.0.1:porta`.
- O status `proxy==1` correspondeu a Relay nos probes; **Direct não foi demonstrado**. `P2pIsActive` não implica Direct.
- Lifetime do túnel por dispositivo é maior que o do RealPlay; não encerrar túnel quando o último tile fecha.
- O bootstrap `t2u_init(server,1250,key)` foi observado, mas servidor/credential/ABI/licença **não foram fornecidos como contrato utilizável**. Endpoints históricos e segredos não entram no código.

**Código acrescentado (ainda sem build local validado):**
`sentrium-device/cloud.hpp` e `cloud.cpp` estabelecem request Cloud com service port explícita, um endpoint local somente após túnel pronto, `CloudRoute::Unknown` por padrão e interface abstrata de provider/túnel. O método `netSdkLogin` cria login local `127.0.0.1:local_port`, **nunca** envia Cloud UUID/remotePort como host/porta NetSDK. Incluído teste focal D-8 para essas invariantes. **Nenhuma implementação P2P/Relay real foi declarada**, pois ainda faltam bootstrap e protocolo/autorização de distribuição independentes; não se admite tunnel mock como funcionalidade real.

## Checkpoint C++ — ciclo de vida Cloud separado do NetSDK (25/10/2026)

- Evidência anterior do operador: CMake configure/generate **PASS**, Debug build **PASS `[44/44] Linking CXX executable sentrium-client.exe`** no commit `c999e72b4da3e621249493dbd19f1ddfcf334de2`. O warning `WrapVulkanHeaders` não bloqueou. Trata-se de compilação, **não de conexão Cloud**.
- Incremento após essa evidência: `CloudDeviceSession` faz a sequência `CloudTransport.open(request)` → endpoint local do túnel → `Adapter.connect(LoginRequest{127.0.0.1, localPort,...})` → preserva sessão NetSDK e túnel durante o uso do dispositivo.
- Na desconexão/destruição, solicita `NetSDK logout` **antes** de `CloudTunnel.close`. Se a inicialização ou o login falhar, fecha o túnel pendente sem criar estado Connected.
- Retorna `CloudRoute::Unknown` até que o provider observe uma rota real; um túnel aberto não comprova modo Direct.
- Teste unitário **D-8** cobre separação entre servicePort remoto e localPort, além da sequência de abertura/encerramento com doubles isolados. Doubles **não** são provider Cloud de produção.
- Estes incrementos precisam ser compilados localmente no Windows antes de receberem PASS. O `CloudTransport` concreto não existe; **Cloud real segue BLOCKED** pelo bootstrap/serviço/contrato independente, conforme o documento de descobertas consolidadas.

## Entrega de UI operacional — Configurações > Registro Automático (26/10/2026)

Após PASS de `D-8` (0,06 s) e build Debug `[12/12]` reportados pelo operador, o trabalho evoluiu para **funcionalidade visível de dispositivos**, sem repetir benchmark:

- Criado acesso `Configurações` no cabeçalho do cliente Qt; a seção `Registro Automático` concentra IP bind, porta, ativação e parada do listener.
- `Adicionar dispositivo > Registro Automático` contém apenas nome, ID de registro, usuário e senha. Cadastrar não abre automaticamente porta de rede.
- Propriedades de estado expõem `registrationListening`, diagnóstico, último evento sanitizado e contador de eventos; o worker diferencia registro de dispositivo conhecido, ID desconhecido e desconexão. Não registrar ID recebido, IP ou credenciais nos eventos visuais.
- Controles de binding ficam desabilitados durante escuta ativa; a configuração é mantida **somente em memória**.
- Corrigido desligamento do listener para respeitar o retorno de `NetSDK logout`; não declarar desconexão bem-sucedida quando o SDK retorna falha.
- Alinhado limite de ID a `szDeviceID[32]` observado no NetSDK: até 31 caracteres utilizáveis mais terminador, sem truncamento silencioso.
- **Código versionado mas ainda não compilado/observado no Windows após essas alterações.** Validar com build Debug focal, abrir Configurações e, quando o gravador estiver disponível, comprovar recebimento de ID, login e canais reais. Esse gate não aprova Cloud nem declara F005 concluída.

## Prioridade atual — conectividade real (26/10/2026)

**Diretriz do operador:** estética da Home por botões menores, ícones quadrados arredondados e nomes abaixo fica **adiada** (referência visual fornecida). A prioridade é **Registro Automático e Cloud/P2P independente do SIM Next**.

**Registro Automático — correções de ciclo de vida (código, ainda sem prova com NVR real):**
- Desconectar sessões NetSDK **antes** de fechar o listener AutoRegistration. Se `CLIENT_Logout` falhar, o sistema não deve informar desconexão bem-sucedida nem abrir sessão duplicada no registro seguinte.
- Guardar temporariamente handles cuja liberação falhou e tentar liberação novamente ao receber registro do mesmo ID; o estado online exibido ao operador só decorre de login NetSDK confirmado.
- A janela de Configurações registra se ID foi reconhecido, se ocorreu login NetSDK e número **real** de canais, ou falha; não mostra ID, senha ou IP nos eventos visuais.
- No teste prático, `bind=127.0.0.1` aceita somente conexões locais. Para gravadores externos, o operador deve configurar um IP de escuta acessível e rede adequada; o cliente não abre firewall/NAT.

**Cloud — impedimento concreto identificado:** as descobertas consolidadas comprovam o fluxo túnel TCP local → NetSDK e Relay no legado, **mas não entregam protocolo de bootstrap, contrato independente e credencial de aplicação autorizada para a infraestrutura Intelbras**. Não usar servidores capturados ou credenciais privadas do SIM Next como solução. Próxima dependência externa é obter da Intelbras contrato/SDK de integração P2P autorizado para VMS C++ independente; há canal público de atendimento para 'Obter API/SDK' divulgado pelo fabricante. O NetSDK genérico ou outras plataformas Cloud de automação não são assumidos como substitutos do P2P de CFTV. Até lá, Cloud **BLOCKED**, não conectado.

**Gate operacional proposto:** após um build focal, testar **uma conexão real por Registro Automático** (listener ativo, evento de registro reconhecido, autenticação, quantidade de canais, desconexão/reconexão); sem repetir os benchmarks F004. O código recente ainda requer esse build Windows.

## F005 Cloud pilot R1 — realistic standalone tunnel candidate (2026-10-26)

**User-reported field result:** operator supplied two screenshots: NetSDK `AutoRegistration` device marked `Connected`, reporting 8 channels, and an active registration listener. The last listener event says "ID desconhecido" but it is an aggregate last event and does not invalidate the independently observed connected recorder. This supports functional AutoReg on **one** recorder; complete lifecycle tests and device-model diversity remain pending.

**Independent P2P discovery:**
- `undervolter/dh-fwd` is a publicly available MIT-licensed Go implementation for `easy4ipcloud`, pinned to `c26ce4f4b7051bfd0b8d07d8a22400f52cb5142c` for this proof. It documents TCP forwarding of arbitrary device ports (including 37777), Direct/Relay variants and authentication for post-2024 devices.
- This source is a *technical pilot*, not proof of compatibility with Intelbras NVRs or legal permission to redistribute or operate a vendor's Cloud infrastructure. Third-party source is **not vendored** into SENTRIUM.
- Source authored for pilot: `scripts/windows/sentrium-cloud-pilot-main.go` (environment-based credential bridge, no passwords in OS startup argv), `scripts/windows/sentrium-cloud-pilot.ps1` (operator-scoped launch), `docs/foundation-005/F005-CLOUD-PILOT-R1.md` (manual build and field trial).
- `DeviceController::addCloudBridgeAndConnect` now exposes **experimental Cloud via operator-managed 127.0.0.1 TCP bridge**. The existing NetSDK worker will authenticate to a known loopback port and enumerate real channels if the tunnel is valid. UI explicitly states local external bridge and never claims native provider, Direct or Relay.
- `CloudRoute::Unknown` remains the truthful status; no socket-to-process attestation, managed tunnel lifecycle, auto restart or clean Windows packaging yet. For production, require safe bridge management or native integration, upstream code security review and vendor/cloud policy validation.
- Code changes and scripts are **not yet compiled or exercised in Windows**; `AutoRegistration` success cannot be extrapolated to Cloud.
