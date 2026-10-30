# SENTRIUM VMS — CONTINUAR SEM PERDER O ESTADO

**Fonte atual do projeto:** [docs/HANDOFF-SENTRIUM-CURRENT.md](docs/HANDOFF-SENTRIUM-CURRENT.md)  
**Branch:** `foundation/005-device-connectivity`  
**PR:** https://github.com/servicosxc-xc/sentrium-vms/pull/4

O repositório GitHub, seu histórico de commits e os arquivos versionados são a fonte de verdade. O texto desta conversa não é o único registro. Ao mudar de chat, **copie esta mensagem**:

> Continue o desenvolvimento do SENTRIUM VMS via GitHub, repo `servicosxc-xc/sentrium-vms`, branch `foundation/005-device-connectivity`. **Antes de responder, leia `CONTINUE-HERE.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` no próprio GitHub e consulte o commit HEAD mais recente**. Desenvolvimento C++/Qt direto no GitHub, sem Codex, em blocos grandes. AutoRegistro real já conectou um NVR Intelbras com 8 canais. Cloud/P2P possui C++ nativo de parser DHP2P, UDP, sinalização, framing PTCP, autenticação do dispositivo via BCrypt (MD5/HMAC/PBKDF2/AES-OFB), porém **falta bootstrap autorizado da infraestrutura, sessão PTCP confiável e túnel real**; não usar SIM Next nem terceiro `dh-fwd` bloqueado pelo Defender. Última compilação **PASS [46/46] + D-9/D-10 PASS** (evidência real do operador em 26/10/2026, commit 4185a700). Problemas antigos de LNK1168 e PRE-FLIGHT foram superados. Para validar incrementos FUTUROS há `scripts/windows/sentrium-validate-f005-cloud.ps1`. **Não reinicie etapas, não repita perguntas respondidas, não invente resultados, não faça merge na main; atualize handoff após os avanços.**

## Objetivo imediato

1. **Validado:** lote C++ WinSock/parsing foi compilado em Debug [46/46] e D-9/D-10 PASS no Windows no commit 4185a700. **Não repetir esses testes sem alterações relacionadas.**
2. **Novo lote PTCP/CNG implementado mas ainda sem build Windows**. Script de validação atual executa D-11/D-12; D-9/D-10 já passaram no commit 4185a700. Continuar implementação autônoma de bootstrap/handshake/PTCP completo somente com dados autorizados; não afirmar login real sem teste com gravador.
3. Quando conexão Cloud real existir, acessar NVRs remotos 16/32 canais e só então avançar no mosaico e em benchmarks representativos.
4. Melhoria estética da Home (ícones menores arredondados/nome abaixo) fica **adiada**.

**Não colocar usuários/senhas/seriais Cloud em documentação ou logs.**

O handoff histórico `docs/HANDOFF-SENTRIUM-2026-10-25.md` cobre F004; não é o ponto atual. O **handoff CURRENT** acima foi criado especificamente para evitar perda de progresso entre conversas.

**Documento do lote atual:** [F005-CLOUD-PTCP-AUTH-R2.md](docs/foundation-005/F005-CLOUD-PTCP-AUTH-R2.md). **Handoff corrente:** seção 13. Use os arquivos reais e HEAD do GitHub ao iniciar outro chat. Não repetir a bateria anterior de testes.

## Checkpoint de transferência de contexto — 26/10/2026

**Importante:** a última versão no GitHub inclui **F005 R2, PTCP framing + criptografia CNG/BCrypt do NVR**, em código C++ ainda **sem compilação/testes Windows confirmados**. O último build comprovado foi **Debug [46/46] + D-9/D-10 PASS** no commit `4185a700`, cobrindo o lote anterior.

Durante a revisão do handoff foi detectado e **corrigido** um erro crítico no script de validação: havia um `ctest -R` truncado e um segundo bloco duplicado no final. Correção versionada em `3de0980fb5397503c06c5fe38cab509c75913676`; documento de estado completo atualizado posteriormente. O script agora se limita a **D-11/D-12** e ainda precisa ser executado no Windows. Não confundir revisão estática com PASS.

### Uma mensagem para colar na próxima conversa

> Continue SENTRIUM VMS. Repositório privado `servicosxc-xc/sentrium-vms` no GitHub, branch `foundation/005-device-connectivity` e PR #4 draft. Leia **integralmente** `CONTINUE-HERE.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` (especialmente as seções 13 e 14), além de `docs/foundation-005/F005-CLOUD-PTCP-AUTH-R2.md`, e confira o HEAD atual. **Não peça novamente o ZIP ou detalhes já registrados.** AutoRegistro NetSDK já conectou um gravador real de 8 canais. O módulo Cloud C++ possui wire/UDP, PTCP framing e criptografia do dispositivo, mas Cloud REAL continua sem bootstrap autorizado, Direct/Relay ou túnel e não deve ser chamado de pronto. Defender bloqueou dh-fwd; jamais contornar. O código R2 ainda precisa de build Windows e testes D-11/D-12; script único `scripts/windows/sentrium-validate-f005-cloud.ps1` já foi reparado. Sem Codex, sem merge na main, sem aprovar Human Review. Implemente em lotes grandes e só dê um comando focal de validação ao fim, atualizando handoff/commits do GitHub.

**Fonte autoritativa:** o conteúdo/versionamento no GitHub, não um resumo de memória do ChatGPT. Para preservar precisão histórica, distinguir claramente **implementado / compilado / testado em campo / não comprovado**.


## R3 — checkpoint posterior (26/10/2026)

Nova camada C++ `cloud_ptcp_session.*` para contadores PTCP, ACK, anti-duplicata, detecção de lacunas, retransmissão e janela limitada; testes D-13/D-14. Atualização de CMake e script focal para **D-11 a D-14**. **Nenhum build Windows deste lote ainda.** D-9/D-10 seguem PASS histórico, sem repetição. Cloud real ainda sem bootstrap autorizado, Direct/Relay e túnel NetSDK. Ver §15 do handoff e `docs/foundation-005/F005-CLOUD-PTCP-SESSION-R3.md`. PR #4 permanece draft.


## R3 runner — ParserError reparado apos retorno Windows (26/10/2026)

O script R3 publicado no commit anterior estava corrompido: CTest regex truncada e bloco finally duplicado. O Windows abortou no parse; nao fez build nem D-11 a D-14. O script foi reescrito na branch em um unico bloco e passou apenas por checagens estaticas. A copia local quebrada exige PRIMEIRO git pull --ff-only origin foundation/005-device-connectivity, DEPOIS executar o script corrigido. Ver secao 16 do handoff. Nao repetir D-9/D-10, F004, nem afirmar Cloud real.


## R2/R3 validados; R4 pendente (26/10/2026)

**PASS COMPROVADO:** commit eb7317c9ce7b088d58cda2c3b883b3ced0dda8fa, PRE-FLIGHT e Debug build [48/48], D-11/D-12/D-13/D-14 4/4 PASS (log real Windows do operador). D-9/D-10 ja tinham PASS. NAO repetir.

**NOVO R4:** API CloudByteStream + TCP dinamico de loopback 127.0.0.1/porta OS unica, encaminhamento de bytes com upstream autenticado pelo provider (ainda inexistente), encerramento cooperativo, D-15/D-16 (so teste echo sinteticamente em memoria). R4 sem build/testes Windows. Script unico agora executa SOMENTE D-15/D-16. Handoff secao 17, documento F005-CLOUD-LOOPBACK-BRIDGE-R4.md. Cloud remoto 16/32, bootstrap autorizado, Direct/Relay e tunel real ainda NAO COMPROVADOS. Manter PR #4 draft, main intacta, sem SIM Next nem dh-fwd.


## Novo checkpoint — R4 Windows aprovado, R5 aguardando compilacao (26/10/2026)

CONFIRMADO R4 no Windows pelo operador: PRE-FLIGHT PASS, Debug build [44/44] PASS, D-15/D-16 PASS 2/2 (0.08 s) em c30d25c65b0331dac0006c000712453e83edc84b. Historico D-9..D-14 tambem PASS. Nao repetir esses testes aprovados.

NOVO R5: codigo C++ de CloudStreamTunnel liga a ponte TCP local ao contrato CloudTunnel/CloudDeviceSession; logout falho do NetSDK agora retém sessao + tunel para retry. Novos testes D-17 e D-18 exercitam somente fixtures TCP locais (16/32 canais FICTICIOS) e logout/retry; D-8 incluso no gate apenas porque cloud.cpp foi alterado. Script unico valida D-8/D-17/D-18. R5 Windows NOT RUN. Referencia: handoff secao 18 e docs/foundation-005/F005-CLOUD-STREAM-TUNNEL-R5.md. Nenhum Cloud remoto real, bootstrap autorizado ou NetSDK NVR remoto comprovado. Main preservada; PR #4 draft, Human Review nao aprovado. Nao usar SIM Next/dh-fwd.


## Checkpoint 26/10/2026 — R5 PASS Windows, R6 pendente

R5 (commit 97159ea2c0e0a274f68f5e7f4066c194a4462a6a) comprovado no Windows: PRE-FLIGHT, CMake/Debug build PASS e D-8/D-17/D-18 3/3 PASS (0,10 s). O log exibe build resumido sem contador [N/N] final. Todos D-9 ate D-16 foram aprovados antes. Nenhum Cloud real foi autenticado.

R6 adiciona PtcpReceiveWindow para buffer PTCP LMID fora de ordem (incluindo controles), rollover de contador, replay e limites de 32 pacotes/256KiB; D-19/D-20 com testes offline; runner passa a executar somente D-19/D-20. R6 codigo COMMITADO, build/testes Windows NOT RUN. Ver handoff secao 19 e docs/foundation-005/F005-CLOUD-PTCP-REORDER-R6.md. Continuam sem autorizacao bootstrap do Cloud, Direct/Relay, stream confiavel ou NVR 16/32 real via Cloud; manter UI Cloud bloqueada, PR draft, main intacta, sem SIM Next/dh-fwd.


## New workflow + R7 checkpoint — 26/10/2026

User requests bigger coherent implementation batches and ONE Windows build at the end, not one test cycle per small class. R6 build on c52f2e9 FAILED with C4456/C2220 in tests/device/main.cpp line 789 (local id hides main id; /WX); D-19/20 did NOT run. Fixed to messageIndex.

R7 integrated post-auth PTCP data plane joins packet fragmentation, LMID ordering, PID/cumulative ACK, heartbeat, retries, backpressure and fail-closed errors. New D-21..D-26 and previously unrun D-19/20 in a single script gate; no re-run of approved D-8..D-18. The script auto detects pinned Qt/NetSDK/PlaySDK roots when missing but PRE-FLIGHT still verifies. R7 build and D-19..D-26 NOT RUN. Handoff section 20, document docs/foundation-005/F005-CLOUD-DATAPLANE-R7.md. Vendor Cloud still blocked by independent bootstrap authorization and live peer/Direct/Relay negotiation. No actual remote NVR 16/32 session.


## Novo checkpoint R7 PASS, R8 aguardando Windows (26/10/2026)

R7 commit 2757ab6eab944b2c1953ad152532b0244e94c057: PRE-FLIGHT PASS, Debug build [44/44] PASS, D-19 ate D-26 8/8 PASS 0,13 s. R8 acrescenta PtcpVerifiedByteStream e interface VerifiedPtcpDatagramLink para encaminhamento PTCP bidirecional ao CloudLoopbackBridge/NetSDK endpoint local, com controle de memoria, backpressure, retries e erros. Testes simulados D-27 a D-32, incluindo loopback SOCKET real mas sem qualquer conexao Intelbras Cloud. Runner unico executa somente D-27..D-32. R8 BUILD/TESTES Windows NOT RUN. Ver handoff secao 21 e docs/foundation-005/F005-CLOUD-DATAGRAM-STREAM-R8.md. Continuam ausentes bootstrap autorizado, peer/Direct/Relay e NVR 16/32 real. PR #4 draft, main e Human Review intactos, sem SIM Next/dh-fwd.


## R8 gate final ampliado (26/10/2026)

R8 foi reforcado para impedir std::terminate em callbacks noexcept se houver excecao durante fila/ACK/retry; retorna falha e fecha o link. Testes D-33/34 cobrem limites e erro de datagrama, alem de D-27..32 ja escritos. Script unico valida D-27..D-34 em Debug. NAO houve build Windows de R8. Handoff secao 22 prevalece para o gate mais recente. PR #4 draft, main nao alterada.


## R8 focused Windows performance change

F005 runner now builds only sentrium-device-tests plus transitive sentrium-device C++ library instead of Qt UI/media/server each round. CTest D-27..D-34. This is a native module gate, not a complete application build; PRE-FLIGHT remains mandatory. See handoff section 23. R8 Windows NOT RUN.


## R8 runner self-reload fix (26/10/2026)

Real operator log: starting old R7 runner on commit 2757ab6, git pull advanced to 881085a, full Debug build [44/44] PASS, but old in-memory script ran D-19..D-26 8/8 PASS (0.12s). R8 D-27..D-34 NOT RUN. Cause: PowerShell loads script before its internal pull. Updated Git runner compares own HEAD blob before/after pull and automatically enters new disk copy with internal SkipPull once if changed. Next operator command is just .\scripts\windows\sentrium-validate-f005-cloud.ps1; local disk copy already selects D-27..D-34. See handoff sec 24.


## Checkpoint 26/10/2026 — R8 Windows PASS, R9 rede UDP nativa pendente

R8 no commit 1294f5c482071e41cb6c0adb3676ef2c7c8ed94b: PRE-FLIGHT PASS, Debug build do sentrium-device-tests PASS e D-27..D-34 8/8 PASS (0,09 s). Nao repetir esse gate.

R9 implementa PinnedUdpSocket nativo Windows com IPv4/porta fornecidos por futuro provedor autorizado, bind efemero, connected-UDP peer pinning, deadlines, limite de datagrama e close concorrente, D-35..D-40 com sockets UDP REAIS localhost. Nao e Cloud Intelbras operacional, nao autentica peers nem implementa VerifiedPtcpDatagramLink. CMake e runner agora D-35..D-40; R9 WINDOWS NOT RUN. Documento docs/foundation-005/F005-CLOUD-UDP-SOCKET-R9.md, handoff secao 25. Main e Human Review intactos, PR #4 draft.

## 27/10/2026: R9 Windows PASS; R10 rede ponta a ponta em laboratório PENDENTE

Operador comprovou R9 no commit 0f639c3e9638dcbd17850c0b118869032945fc2b: PRE-FLIGHT e Debug build focal PASS, D-35..D-40 **6/6 PASS** 0,26 s. O runner fez reload automático após pull e trabalhou corretamente. Não repetir.

R10 cria somente o executável de testes `tests/device/cloud_network_lab.cpp` e alvo `sentrium-cloud-network-tests`, usando sockets Windows **TCP e UDP reais em 127.0.0.1**, PTCP R7/R8, PinnedUdpSocket R9 e CloudLoopbackBridge R4 com um respondedor de laboratório **sem autenticação**. Testes D-41..D-46 cobrem eco fragmentado, TCP→UDP→TCP, fonte externa filtrada, ACK/realm inválidos, retransmissão de UDP perdido e fechamento simultâneo. Runner focal passa a D-41..D-46. **R10 NOT RUN Windows** até log. Ver handoff seção 26 e docs/foundation-005/F005-CLOUD-NETWORK-LAB-R10.md.

Continua BLOQUEADO o provedor Cloud Intelbras por serial: falta app identity/bootstrap/peer auth/Direct/Relay autorizado e ensaio NVR 16/32 real. O adaptador inseguro UnverifiedLabLink é SOMENTE fixture no executável de teste, NUNCA componente de produção. Main intacta, PR #4 draft, Human Review pendente. NÃO usar SIM Next nem dh-fwd.


## 27/10/2026 — R10 6/6 PASS Windows; fornecedor Cloud gate

R10 comprovado no commit c211b964eefea6caa9f74c88bba9095ef9984e6d: PRE-FLIGHT PASS, build Debug sentrium-cloud-network-tests [8/8] PASS, D-41..D-46 6/6 PASS em 1,38s. Integração UDP/PTCP/TCP loopback sem autenticação externa. Novo documento docs/foundation-005/F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md pesquisa Intelbras e contém solicitação pronta SDK Cloud P2P por serial. Até fornecedor autorizar aplicação/peer/Direct/Relay ou usuário optar por gateway remoto seguro, Cloud por serial bloqueado. Não iniciar novas simulações. Main intacta, PR draft, Human Review não aprovado.


## RE-001 — Interoperabilidade SIM Next em modo observacao (27/10/2026)

Usuario aprovou pesquisa de interoperabilidade por observacao de instalacao legitima SIM Next e DVR/NVR autorizado; licenciamento comercial a tratar formalmente quando houver definicao viavel. Ferramenta scripts/windows/sentrium-observe-simnext.ps1 cria relatorios JSON locais sob out/simnext-interop, sem capturar payload nem credenciais. Detecta processos e filhos, nomes DLL, assinaturas/hash, TCP IPs pseudonimizados com salt local e portas UDP locais; Get-NetUDPEndpoint nao mostra peer UDP. A UI SIM Next classifica IP/Dominio, Cloud P2P ou Cloud Relay. Manual de etapas e stop/go: docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001.md. Observer Windows NOT RUN; nenhuma sessao real coletada. R10 permanece 6/6 PASS, sem novas mudancas C++ nem necessidade de build. Proximo comando no PC com SIM Next aberto, repo root: .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Connecting ; ao pedir ENTER, iniciar conexao no SIM Next. Nao compartilhar salt/PCAP bruto. Nunca copiar chaves, burlar validacao, rodar dh-fwd bloqueado ou desativar Defender. PR #4 draft, main e Human Review intactos.


## RE-001 Observation 01 actual metadata analyzed, 27/10/2026

The user supplied the Connecting JSON plus the salt file. Verified SIMNext.exe 1.25.4.1203 PID 22076, 196 loaded library names, 25.2 s. Of 411 grouped rows, 322 TCP/89 UDP; TCP split: 161 IPv4 unspecified binding/listens, 1 IPv6 unspecified, 146 loopback and 14 external. 144 loopback Established rows are 72 reciprocal socket pairs. TCP remote-port 37000 showed 3 SynSent sequences; 37777 had one pre-existing Established and 2 new SynSent attempts. UDP peers NOT observable. No P2P versus Relay determination or specific NVR correlation without the operator's UI confirmation and comparison captures. The uploaded salt MUST NOT be checked in; regard it exposed, rotate before a newly shared series if privacy matters. Sanitized report docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-01.md. Next capture is Idle with no camera tiles open, then Live. No C++ build required; R10 PASS stands.


## RE-001 OBS-02 — Connecting vs Idle real differential (27/10/2026)

Second operator JSON Idle is analyzed against Connecting. Same SIMNext.exe v1.25.4.1203 PID 22076. TCP grouped rows 322 vs 500; UDP LOCAL ports 89 vs 112; TCP local/wildcard 308 vs 470; TCP nonlocal remote rows 14 vs 30; SynSent 5 vs 20; established reciprocal localhost pairs 72 vs 91. Exact 5 long-lived off-host TCP socket tuples present both runs to service ports 7000, 12246, 37001, 37719, 37777. Port 37000 SynSent 3 vs 8; 37777 SynSent 2 vs 8 + same Established in both; 37002 0 vs 4. UDP overlap 72, Connecting-only 17, Idle-only 40. These are SOCKET METADATA NOT packet volume, NAT peer, Direct/Relay, authentication, or NVR identification. Salt is uploaded separately; NEVER publish it/endpoint tags/raw report. Sanitized docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-02.md. Next experiment: script -Scenario Live then open precisely ONE remote DVR/NVR camera EXTRA after ENTER; record UI mode and outcome. No build required. R10 Windows 6/6 PASS. PR draft, main intact.


## RE-001 OBS-03 — Actual Live cross-comparison (27/10/2026)

Actual SIM Next Live JSON 18:43:49 UTC, 25.99s, same pid 22076. Live TCP446 UDP104, off-host TCP20, 11 SynSent, 91 reciprocal localhost Established pairs. Compared with Idle TCP500 UDP112 off-host30 SynSent20 pairs91 and Connecting TCP322 UDP89 off-host14 SynSent5 pairs72. **Live 535/550 rows already present first sample, incl all 104 UDP sockets and all nine off-host Established TCP tuples.** All nine existed in Idle too (ports 7000,12246,15832,28295,37001,37719,37777,44431,57314). 37000/37002/37777 SynSent retries also in Idle; NOT proof Cloud traffic. No new off-host TCP established later, but UDP stream entirely unobserved. Stop repeating metadata-only captures. Next RE-002 one passive Wireshark capture ~10s idle +20s exactly one EXTRA camera +10s closed; record successful picture and UI P2P/Relay/IP status, inspect Conversations UDP by Bytes. Do NOT share raw PCAP, salt, IPs or tokens. See docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-03.md. No rebuild; R10 Windows 6/6 PASS, PR draft and main intact.


## RE-002 OBS-01 (27/10/26): Wireshark identificou candidato FORTE stream UDP

User enviou screenshot (aba TCP 33, UDP tab indica 234) e exportacao texto de 4707 linhas, tempo 0..13.157s. Analise local extraiu UDP 4424 pacotes / 2,369,454 B. UMA conversa UDP (porta remota 1639/local 64220) domina: 1737 pacotes, 2,007,020 B; inbound 1546 / 1,994,186 B, outbound 191 / 12,834 B; ~84.7% dos bytes UDP da amostra, ~1.8 Mb/s inbound a partir t=4.656s. Fluxo antes t=4s somente 100-1000 bytes/seg, depois 206-239 kB/s, 1197 pacotes 1442 B. FORTE candidato a video, mas **NAO prova Cloud Direct/Relay nem endpoint NVR; UI/video nao confirmados**. Amostra dura 13.16s, NAO inclui encerramento e seu screenshot mostrava aba TCP, nao UDP. Documento SANITIZADO em docs/foundation-005/F005-SIMNEXT-INTEROP-RE-002-OBS-01.md, sem IP/serial/raw payload. Proximo: captura ~40s (10s sem camera,20s uma camera extra,10s fechada), revisar UDP Conversations e filtro local udp.port == 1639 && udp.port == 64220; enviar screenshots redigidos/IO Graph e informar estado UI. Sem C++ build; R10 PASS, main/PR draft preservados.


## RE-002 OBS-02 (27/10/26): Full PCAPNG 49s reveals real UDP envelope

User sent real ~7.2MiB PCAPNG, parsed offline: Ethernet, 15,087 frames / 48.957s. Dominant UDP local port64220, remote port1639: 4508 frames / 5,484,597B, inbound 4068/5,456,667B, outbound440/27,930B. First >=900B packet t13.913s, last t36.482s; sustained ~1.92Mb/s t14..36 then stops, strongly correlates with planned camera live interval but UI/video success not yet confirmed. Of all UDP, type-prefix hex 12011000=1725, 12011001=1710, 12011002=6481, 12011003=2641. ALL 6481 type02 UDP payloads obey 16-byte header length field at offsets8:10; 4035 inbound main-flow type02 frame IDs contiguous (no gaps/duplicates), 405 outbound short type03 frames reference those IDs (ACK-like). **NO UDP payload contains ASCII PTCP**; our current cloud_ptcp.cpp expects PTCP + 24B and cannot parse observed 1201 envelopes directly. Do not declare protocols equivalent or discard separate inner layers. Need actual SIM Next UI mode P2P/Relay/IP and app/device bootstrap, not another synthetic UDP test. Sanitized report: docs/foundation-005/F005-SIMNEXT-INTEROP-RE-002-OBS-02.md. No raw PCAP/IP/secret repo. No code build; R10 6/6 PASS, PR draft/main safe.


## Operador confirma P2P no RE-002 (27/10/26)

Captura comecou sem camera, depois abriu um canal e fechou; conexao rotulada P2P pela UI do SIM Next. Correlaciona media UDP 1639 ↔ 64220 t13.913-36.482s com abrir/fechar em sessao P2P rotulada. **Nao comprova rota fisica diretamente no NVR nem login independente no Cloud.** Proxima pesquisa RE-003: captura Wireshark com aplicativo SIM Next fechado no inicio, comecar Wireshark, abrir SIM Next normalmente, observar dispositivos e UMA camera extra, registrar tempos e P2P UI; manter PCAP local, sem extrair segredos. Handoff secao 34, obs02 doc revisado, PR #4 draft/main intactos, R10 6/6 PASS.


## RE-003 OBS-01 (28/10/2026 UTC) — Authoritative site WAN IP matches BOTH P2P video PCAPs

Operator supplied actual current public WAN IP of authorized DVR site. Offline PCAP matching shows EXACT same public IP as remote UDP video endpoint on **BOTH** capture1 (port1639 ↔ local64220, 4508 frames 5.484597MB) and capture2 startup (first probe remote1132 t40.767, then remote1131 ↔ local51699 from t41.369, 3964 frames 3.586029MB). DO NOT write user-provided numeric IP to GitHub. SIM Next UI confirms P2P. This is STRONG evidence direct-to-recorder-SITE P2P (not a vendor relay transporting the observed media), but NAT/router may forward to DVR internally; does NOT prove third-party Cloud auth. Startup DNS lookups include intelbrasp2p.com.br, api.simnext.isec.intelbras.com.br, api-cronos.intelbras.com.br, cronos-simnext.intelbras.com.br around t11–14; local UDP rendezvous packets at t40.77–42.16 with observed 12 20 D0/D1/D2 and then 12 01 10 xx, bulk media at ~t56 to70. Exact DNS role/app auth not known. SANITIZED doc docs/foundation-005/F005-SIMNEXT-INTEROP-RE-003-OBS-01.md. No raw PCAP/IP/credential committed. C++ R10 6/6 PASS stands; PR draft, main/Human Review unchanged.


## RE-004 — DNS/TCP + UDP pre-media negotiation audit implemented (28/10/2026)

Added reproducible metadata-only PCAPNG analyzer at scripts/research/sentrium-p2p-pcap-audit.py, synthetic tests (4/4 PASS on analysis runtime), PowerShell wrapper scripts/windows/sentrium-audit-f005-pcap.ps1 (Windows NOT RUN). Both real operator PCAPs analyzed offline with correct frame count/UDP leading ports and 16-byte envelope consistency (6481/6481 and 3254/3254). Fresh SIM Next startup: DNS query/HTTPS SYN pairs for api-cronos t11.252/11.260, api.simnext.isec t12.641/12.662, logs.isec t14.423/14.429; site UDP precursor remote1132 t40.818 then 1131 t41.420 and D0/D1/D2, 00/01, 05/06, 02/03. RE-003 duration re-audited **73.689s**. No sensitive user IP/payload/credential in repository. **No authorized Cloud app bootstrap yet**; don't wire existing PTCP parser to observed 16B envelope or falsely claim login. Details docs/foundation-005/F005-SIMNEXT-INTEROP-RE-004-METADATA-AUDIT.md. Python tests 4/4 local PASS, Windows wrapper not run. No C++ changes; R10 6/6 PASS, PR draft, main untouched.


## RE-004 Windows R1 hotfix runtime Python — 28/10/2026

Real operator test failed on Windows: `py -3` launcher attempted deleted `%LOCALAPPDATA%\Programs\Python\Python313\python.exe`; analyzer never ran, no PASS. PCAP path valid. `scripts/windows/sentrium-audit-f005-pcap.ps1` updated to probe Python executable/version/exit-code, fallback to python/python3, py explicit 3.x and LOCALAPPDATA versions, optional -PythonPath. If none works fail safely with diagnostics. Last valid R10 C++ 6/6 PASS unaffected. Next test operator exact PCAP path `C:\Users\ericx\Downloads\wireshaark 2.pcapng`, run git pull + wrapper. No new PCAP or C++ build. RE004 doc appended. PR draft/main/F004 Human Review preserved.


## RE-004 TShark fallback Windows (28/10/2026)

Operador validou TShark diretamente no Windows: 3964 pacotes 3586029 bytes na captura RE-003. Sem Python funcional; wrapper RE-004 agora cai automaticamente para modo TShark UDP IPv4, grava JSON sanitizado somente em out/simnext-interop, nao grava IP/payload, declara PASS PARCIAL DE TRANSPORTE (nao autenticao Cloud, DNS ou parsing de envelope). Novo wrapper TShark precisa de uma unica validacao Windows usando exatamente mesmo comando. Sem nova captura, sem instalar Python, sem novo build C++.

## RE-004 TShark Windows validado (28/10/2026)

Log do operador apos pull do commit ed67873f: RE-004 TSHARK PASS PARCIAL com 3964 pacotes / 3586029 bytes, correspondentes a referencia RE-003. JSON sanitizado criado localmente em out/simnext-interop/re004-tshark-transport-20261028-041303-716.json (arquivo nao recebido/inspecionado); Python auditor completo NAO executado. Nao repetir captura nem exigir Python. Fontes oficiais Intelbras revisitadas: forum 2018 SDK Cloud por serial distinto do IP/DDNS e nao oferecido naquele ano; orientação 2024 SDK/API convencionais via canal suporte; manual Guardian Parceiro v1.6.0 documenta Cloud P2P/Relay como aplicativo oficial mas nao oferece SDK publico terceiro. Gate ainda resposta da Intelbras a consulta de autorização por serial em docs/foundation-005/F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md. R10 Windows C++ 6/6 PASS permanece, main/F004 Human Review intactos e PR4 draft.


## Decisao F005 — sem SDK Cloud do fabricante via chatbot (28/10/2026)

Usuario confirma chatbot nao fornece SDK Cloud SIM Next; so NetSDK DVR/NVR que ja foi entregue. **Nao propor repetidos contatos ao chatbot**. Nova direcao em docs/foundation-005/F005-NO-VENDOR-CLOUD-SDK-PLAN.md: executar AutoRegistration NetSDK (ja observado em NVR real 8ch) e suporte de rede remota segura, com IP NetSDK via WireGuard como alternativa. Nao confundir AutoRegistro configurado no gravador com Cloud via serial; AutoRegistro remoto depende de endpoint servidor alcançavel e configurado. P2P Intelbras independente continua pesquisa passiva e BLOCKED em autenticacao/bootstrap; nao copiar app secrets. Codigo C++ inalterado, R10 6/6 PASS ultima evidencia. PR #4 draft/main/F004 Human Review preservados.


## Cloud lib lab LAB-001 — 28/10/2026

Usuario decidiu avaliar conexao Cloud com bibliotecas SIM Next ja instaladas antes de formalizar a integracao. Novo experimento isolado em tools/simnext-cloud-lab (.NET 10 x64, via reflexao de VVEyeAdapter e libt2u local) + script scripts/windows/sentrium-simnext-cloud-lab.ps1. **NAO redistribuir/copiar libs**, sem extrair segredos. Iniciar somente -Mode Inspect, selecionando SIMNext.P2PSDK.dll original; nenhum Cloud UUID/credencial e nenhum carregamento de DLL no INSPECT. Modo Connect fica gated por confirmacao local, uso apenas autorizado e prova de escopo do listener. **Nao declarar funcionando**, codigo novo Windows NOT RUN. Detalhes docs/foundation-005/F005-SIMNEXT-CLOUD-LIB-LAB-001.md. A decisao anterior de cloud C++ sem dependencias segue valida para produto; este lab nao altera product code, r10 6/6 PASS precedente, PR draft e main/F004 Human Review intocados.


## LAB-001 INSPECT Windows PASS (28/10/2026)

Operator ran -Mode Inspect after pulling commit 0b0cdf1, dotnet10 compiled and harness returned INSPECT PASS twice. DLL statuses: SIMNext.P2PSDK/Core/Model AMD64 Authenticode Valid; Libt2uWrapper .NET PE I386 NotSigned (I386 is not necessarily 32bit-required: managed AnyCPU check pending); libt2u AMD64 Native NotSigned. Five files present locally; no vendor assembly loaded or Cloud connection attempted in Inspect. Next controlled LAB-CONNECT (if allowed by applicable use terms), using scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode Connect with user-selected SIMNext.P2PSDK.dll original, typed CONFIRMO, local Cloud ID and actual remote service port. Do not enter IDs/secrets in chat. May produce a Relay tunnel only. Stop if unexpected library/security warnings. No product native code changed, C++ R10 6/6 PASS historical, PR4 draft main/F004 untouched.


## LAB-001 CONNECT real — parcial: ACTIVE, bind nao loopback (28/10/2026)

Log do operador do lab confirma wrapper DLL carregado (`ADAPTER_READY`), porta TCP local criada, `P2pIsActive` ACTIVE, `RelayConnection` NOT_RELAY_REPORTED (NAO implica Direct), listener TCP observado, mas `BIND SCOPE NOT_PROVEN_LOOPBACK` -> SAFETY STOP exit9. Cloud ID privado apareceu em chat anterior: **NUNCA repetir esse identificador em docs/repos/logs**. Nao ligar NetSDK nesse endpoint sem isolamento seguro; nao mudar firewall. Hotfix LAB-001 r1 em tools/simnext-cloud-lab/Program.cs: classificar bind em tags sem IP (loopback/wildcard/interface), ocultar entrada Cloud ID para evitar eco no console. Hotfix ainda NAO validado em Windows. Proxima etapa operador roda git pull + `scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode Connect`, digita Cloud ID localmente e compartilha so tags de binding/estado sem IDs. PR #4 draft, C++ main/F004 nao mudou; native R10 6/6 synthetic lab historico preservado.


## LAB-001 CONNECT exit=7 apos antes exit=9; diagnostico R2 (28/10/2026)

Nova tentativa usuario: bibliotecas carregadas ADAPTER_READY, mas AddSDKPort nao retornou porta valida (`TUNNEL NOT_READY`, exit7). Em tentativa anterior SAME lab havia returned local port, ACTIVE, listener observed e safety stop exit9 por bind nao-loopback. Sem causalidade determinada; ultima entrada Cloud ID foi oculta, possivel divergencia nao comprovada. Novo harness faz dupla entrada segura sem eco e MATCH/MISMATCH (mismatch exit11 sem chamar cloud), cronometra unico AddSDKPort e classifica ZERO/NEGATIVE/OUT_OF_RANGE/VALID_PORT, excecoes categorias fechadas exit12. Nao faz auto-retry/alteracao firewall nem ignora STOP. Novo codigo BUILD Windows NOT RUN; operator proximo teste pull + mesmo `-Mode Connect`. Nunca copiar Cloud ID que vazou em log de chat. PR4 draft, main/F004 intactos, native C++ R10 6/6 lab synthetic anterior.


## LAB CONNECT R2 RESULT ACTIVE & WILDCARD_IPV4; READ-ONLY owner probe R3 (28/10/26)

User real log: CLOUD_ID_INPUT MATCH, AddSDKPort VALID_PORT after 1920ms, ACTIVE, NOT_RELAY_REPORTED (not proof direct), TCP LISTENER OBSERVED, BIND CATEGORIES WILDCARD_IPV4 (0.0.0.0), SAFETY STOP exit9. No NetSDK credentials sent. Therefore vendor library opens the tunnel under separate .NET harness but a listener is potentially LAN-exposed and must NOT be used for login yet. New R3 C# adds read-only GetExtendedTcpTable listener OwningProcess categorization (no PID/IP/serial output), still automatic STOP and no firewall changes. R3 build/test Windows PENDING. Next user one command git pull + same -Mode Connect; share LISTENER OWNER and bind categories only. Then isolation design with *separate explicit firewall permission*, do not unilaterally change firewall. PR4 draft, main/F004 unchanged, no product C++ modifications.


## LAB-001 R3 OWNER Windows PASS, bind WILDCARD_IPV4 ainda BLOCKED (28/10/26)

Ultimo Windows log: CLOUD_ID_INPUT MATCH, AddSDKPort VALID_PORT 1921ms, TUNNEL ACTIVE, NOT_RELAY_REPORTED (nao prova Direct), listener WILDCARD_IPV4 e LISTENER OWNER CURRENT_LAB_PROCESS. Portanto a DLL em processo do laboratorio e dona do socket 0.0.0.0; exit9 continua correto. Nenhum NetSDK login realizado. Escrito plano de isolamento `docs/foundation-005/F005-SIMNEXT-CLOUD-LAB-FIREWALL-ISOLATION-PLAN.md`: proposta de regra temporaria de block inbound TCP por executavel somente com aprovacao explicita, execucao da DLL nao elevada e teste em segundo host na LAN, depois login NetSDK; plano NAO executa nada. Nao repetir Cloud ID vazado em conversa. Sem novos testes de Wireshark ou C++ R10. PR4 draft, main/F004 intactos.


## Firewall TEMP lab aprovado e implementado (28/10/2026)

Usuario autorizou regra BLOCK TCP INBOUND exclusiva do Sentrium.CloudLibLab.exe temporaria com rollback. Foi implementado scripts/windows/sentrium-cloud-lab-firewall.ps1 (Prepare, Install UAC, Status e Remove UAC) com Fail Closed se os perfis nao estiverem ativos/politica nao aplicar a regra. Novo -Mode IsolationProbe no lab (nao elevado) usa apphost C# e verifica regra antes de carregar DLL; nao solicita senha, apenas permite checar localhost e teste TCP de outro PC LAN enquanto aguarda ENTER, com tentativa de remover regra via UAC ao sair. Primeiro e UNICO comando necessario agora: git pull + `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action Prepare` em Developer PowerShell normal. Windows Build/Test desse lote NOT RUN. Main/F004 e C++ produto intactos, PR4 draft, login NetSDK nao validado.


## F005 Firewall Prepare: compatibilidade com console elevado (28/10/2026)

Usuario abriu VS Developer PowerShell e PowerShell Win+R, ambos `Is-Admin=True` no Windows, e script antigo Prepare abortou ANTES do build e da regra. Atualizado `scripts/windows/sentrium-cloud-lab-firewall.ps1` para aceitar os dois contextos: log `PREPARE TOKEN ELEVATED/STANDARD`, build/apphost sem carregar Cloud DLL, Install direto caso ja elevado, ou UAC caso nao, seguido por Assert-Active. **IsolationProbe PERMANECE obrigatoriamente nao elevado**; se o Win+R ainda for admin, proximo passo identificar UAC/conta padrao, nunca desativar a verificacao. O novo Prepare ainda precisa de execucao real Windows; nenhuma regra criada pelos erros anteriores. Comando unico git pull + firewall.ps1 -Action Prepare na shell atual. Sem alterar C++ ou main/F004, PR draft.


## LAB firewall Prepare Windows REAL PASS, token elevado (28/10/2026)

Operator puxou 894c1823 e executou Prepare num PowerShell com WindowsPrincipal admin token: build .NET 10 Release OK, regra `SENTRIUM-F005-LAB-TCP-IN-BLOCK` CRIADA, verificada ActiveStore PASS duas vezes, `F005 PROTECT READY`; nenhuma DLL Cloud executada. A regra agora EXISTE no PC ate Remove; nao afirmar que LAN esta bloqueada sem segundo host. Duas shells iniciadas com token elevado, inclusive PowerShell Win+R; nao remover gate do isolamento. Adicionado modo `-Action DiagnoseToken` read-only, mostra UAC enabled/disabled, conta built-in Admin sim/nao, Admin Approval Mode, token atual elevated/standard, SEM login/sid. Proximo passo git pull + DiagnoseToken (nao requer repetir Prepare). Se UAC disabled/builtin Admin, sugerir conta padrao ou ajuste controlado do UAC; nao mudar UAC automaticamente. Main e F004 intactos, PR4 draft.


## Firewall script DiagnoseToken PARSE ERROR fixed in repository (29/10/2026)

User ran DiagnoseToken on 2b41fbd4 and got PowerShell ParserError. The GitHub script itself was broken: malformed `-match '-500` regex inside Diagnose-Token plus duplicated earlier firewall switch embedded in the function, with stray `)`; closing Windows PowerShell was not the cause. Repaired exact corrupt range retaining a single switch and correct `'-500$'` regex. Static structural checks executed at commit; Windows PowerShell native Parser.ParseFile NOT RUN yet. Next user: pull; run Parser.ParseFile with no execution before read-only DiagnoseToken. The temporary TCP inbound BLOCK firewall rule was installed in earlier Prepare PASS and remains until intentional Remove. No Cloud DLL run, no firewall changes in this repair, no main/F004/product changes.


## F005 Windows token conclusao — UAC DISABLED (29/10/2026)

Operator confirmed: Parser PASS, TOKEN DIAG CURRENT_TOKEN ELEVATED, UAC DISABLED, BUILTIN_ADMIN_ACCOUNT NO, BUILTIN_ADMIN_APPROVAL_MODE DISABLED. This is the explanation for why even Win+R PowerShell is elevated; builtin approval mode is not relevant because not using built-in Administrator. NEVER run cloud DLL as admin. Operator can manually enable UAC via Windows User Account Control Settings and reboot, then verify UAC ENABLED and token STANDARD, or use separate standard non-admin account. UAC changes globally and must be controlled by user; no scripts change it. Scoped inbound TCP Block rule was previously installed successfully; leave until further lab or remove with -Action Remove (rule does not open ports). NETSDK login and second-LAN-host validation still not run. Documentation updated in firewall runbook; PR4 draft, product C++ unaffected.


## UAC habilitado e token STANDARD REAL PASS; Git dubious ownership (29/10/2026)

Operador executou DiagnoseToken localmente (sem git pull por dubious ownership): CURRENT_TOKEN STANDARD, UAC ENABLED, builtin_admin NO, builtin approval DISABLED (irrelevante para conta atual). Pronto para rodar Cloud Lab nao elevado. Firewall regra BLOCK previamente instalada; ainda nao reverificado apos reinicio. Git pull falhou por dono do diretorio BUILTIN/Administradores diferente da identidade atual. Sem necessitar mudar safe.directory agora: wrapper IsolationProbe ja esta no checkout desde fc409df6. Proximo passo SEM git pull `scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode IsolationProbe`: ele verifica firewall Status PASS antes de rodar EXE, sem login NetSDK; mostrar porta temporaria e testar de outro PC da mesma LAN (deve False), ENTER tenta rollback UAC. Regras existentes nao alteradas no diagnostico. PR4 draft, main/F004 intactos.


## ISOLATION-PROBE Windows local PASS, LAN pendente, regra removida (29/10/2026)

Usuario confirmou em token STANDARD e UAC ENABLED: regra inbound BLOCK ativa, AddSDKPort VALID_PORT 1947ms, ACTIVE, listener WILDCARD_IPV4 owner CURRENT_LAB_PROCESS, localhost TCP CONNECTED. Ao pressionar ENTER, F005 FIREWALL REMOVED confirma rollback. Nao houve Test-NetConnection de outro PC LAN, portanto nenhum isolamento externo aprovado e nenhum login NetSDK. Para repetir teste de LAN, sem git pull por dubious ownership: executar -Action Prepare para reinstalar a regra, depois -Mode IsolationProbe com outro PC autorizado testando porta temporaria antes do ENTER; so depois avaliar gate NetSDK. PR4 draft, main/F004 intocados.
