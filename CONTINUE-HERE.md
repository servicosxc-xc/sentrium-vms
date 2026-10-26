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
