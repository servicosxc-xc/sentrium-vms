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
