# SENTRIUM VMS — HANDOFF PERMANENTE / ESTADO ATUAL

**Atualizado em:** 26/10/2026  
**Projeto:** SENTRIUM VMS — Intelligent Video Management System  
**Repositório:** `servicosxc-xc/sentrium-vms` (GitHub)  
**Branch ativa:** `foundation/005-device-connectivity`  
**PR de desenvolvimento:** https://github.com/servicosxc-xc/sentrium-vms/pull/4 (draft, sem merge)
**Regra:** após abrir um novo chat, conferir `git rev-parse HEAD` no GitHub antes de prosseguir. O HEAD atual da branch é a fonte de verdade; este texto informa as decisões, não substitui o Git.

> **ÚLTIMO CHECKPOINT CONFIRMADO — 26/10/2026, 17:08 (Brasil):** Commit `4185a7000940c2c31f8dbdd050198f5070429bf1` **PREFLIGHT PASS, CMake configure PASS, Debug build/link PASS [46/46], D-9 PASS, D-10 PASS**. Evidência: log real `Texto colado(20261026-170803).txt` encaminhado pelo operador. As seções 6 e 11 descrevem **erros históricos já superados**, não o estado corrente. Cloud real continua **não implementado/não comprovado**. Veja seção 12.

> **CHECKPOINT NOVO — 26/10/2026:** F005 R2 PTCP + autenticação nativa do dispositivo (CNG) **CODIFICADOS, SEM BUILD WINDOWS DESTE LOTE**; ver seção 13. Última execução confirmada continua `4185a700`: Build Debug PASS [46/46], D-9/D-10 PASS. Próxima validação focal: D-11/D-12. Cloud remoto e Direct/Relay ainda NÃO COMPROVADOS.

> **TRANSFERÊNCIA SEGURA ENTRE CHATS — 26/10/2026:** documentação durável e versionada: `CONTINUE-HERE.md` + este handoff. O último lote de código é F005 R2 **PTCP/CNG ainda sem build Windows validado**, ao contrário do lote R1 D-9/D-10 já aprovado. O script PowerShell único de R2 tinha comando CTest truncado e bloco duplicado; **corrigido no commit `3de0980fb5397503c06c5fe38cab509c75913676`**. Continuar desse ponto, sem reinstalar ferramentas e sem reiniciar Foundations anteriores. Consultar HEAD GitHub novamente ao iniciar o próximo chat.

## 1. Decisão de colaboração — O MAIS IMPORTANTE

O usuário pediu expressamente para **PARAR de fragmentar trabalho em microentregas e executar cada bloco de implementação de uma vez**. Fazer o maior avanço de código seguro/justificável por etapa, escrever todo o lote no GitHub, conferir consistência estática e **entregar UM procedimento de build/teste no final**, não dezenas de comandos intercalados. Não prometer em chat um Cloud completo se faltar contrato real. Não propor testes repetitivos sem benefício concreto.

O usuário também exige **continuidade entre conversas sem perda de decisões/progresso**. Este documento, `CONTINUE-HERE.md`, `docs/foundation-005/` e commits/PR são a memória operacional durável. Atualizar este handoff no fim de cada entrega significativa. Não depender apenas de memória do ChatGPT. No novo chat, ler o handoff + inspecionar branch antes de alterar algo.

## 2. Plataforma e produto

- Aplicação VMS profissional em C++20/Qt 6.12.0/MSVC 2022, NetSDK Intelbras/Dahua, PlaySDK, vídeo H.264/DHAV e futura IA/LPR.
- Nome de produto atual **SENTRIUM**; XC Video IA Foundation 007 r25 é **referência funcional/visual histórica**, não código a copiar indiscriminadamente nem arquitetura C++.
- `main` contém F001–F003 aprovadas; F004 em `foundation/004-media-lab` com PlaySDK/FFmpeg/demux testado; F005 ativa no branch acima. F004 não recebeu Human Review final, não forçar merge nem declarar ADR-005 aprovado.
- **Sem Codex:** desenvolver diretamente via conector GitHub; comandos Windows somente para validação do operador.
- Computador do operador: `C:\Projetos\SENTRIUM VMS\sentrium-vms`, MSVC Developer PowerShell, Qt `C:\Qt\6.12.0\msvc2022_64`, NetSDK `C:\Temp\IntelbrasSDK\NetSDK64`, PlaySDK `C:\Temp\IntelbrasSDK\PlaySDK64`, toolchain fixado pela `scripts/windows/sentrium-preflight.ps1`.

## 3. Estado funcional comprovado em campo

### LAN / NetSDK
Validado nas Foundations anteriores com equipamento Intelbras real e canais retornados.

### Registro Automático
**COMPROVADO EM UM GRAVADOR REAL**: screenshot do operador mostra `NVR Intelbras` **Connected** por `AutoRegistration`, **8 canais retornados**. Outro print mostra listener ativo, IP bind local configurado e 4 eventos; a última mensagem é ID desconhecido, indicando evento extra ou cadastro diferente, não falha da sessão conectada. Ainda não foram comprovados reinícios, perda de rede, ciclo longo e diversidade de modelos.

O Registro Automático usa **ID configurado pelo operador no gravador**, igual ao cadastrado no SENTRIUM, NÃO número de série físico. `DeviceWorker` usa `CLIENT_ListenServer`/NetSDK, login após callback, guarda sessão, tenta logout antes de parar listener e preserva handles quando logout falha para não criar sessões duplicadas. Controles de escuta no módulo Configurações, credenciais somente em memória.

### Interface
Shell Qt com Home, abas estilo navegador (Visualização, Dispositivos, Eventos, Regras IA, Mídia, LPR, Configurações), árvore de dispositivos e lista real. Usuário enviou referência para futuramente mudar cards Home para **botões de ícone quadrado pequeno com cantos arredondados e nome embaixo**; **ADIADO** — Cloud e AutoRegistro são prioridade. Placeholder honesto para módulos ainda não funcionais.

## 4. Cloud — restrições inegociáveis

- **SENTRIUM independente do SIM Next:** nenhuma dependência de instalação, serviços, DLLs `SIMNext.P2PSDK.dll`, `Libt2uWrapper.dll`, `libt2u.dll` ou bootstrap secrets extraídos.
- Os arquivos e a engenharia reversa antiga ajudam a compreender o comportamento **T2U → túnel TCP local dinâmico → NetSDK**. Uma porta observada `37000` era específica de um equipamento, não padrão universal. Relay teve evidência no legado; Direct nem sempre.
- O SDK que a Intelbras enviou foi **General NetSDK/PlaySDK**, não API Cloud independente. O usuário já consultou o fabricante e não obteve Cloud SDK. **NÃO repetir como se fosse uma opção ainda não tentada.**
- O programa `dh-fwd_win_x64.exe` (v0.2) baixado para piloto Cloud foi colocado em **quarentena pelo Defender** (`Trojan:Win32/Bearfoos.A!ml`). A execução foi bloqueada; hash coincidente com release não prova segurança.
- **SECURITY HOLD:** não executar/restaurar arquivo, desabilitar Defender, criar exceções, repacotar/recompilar a ferramenta sinalizada para burlar proteção. Não inserir credenciais em proxy local não verificado.
- `scripts/windows/sentrium-cloud-pilot.ps1` interrompe imediatamente; `Main.qml` Cloud está desabilitado; `DeviceController::addCloudBridgeAndConnect` agora também rejeita mesmo quando chamado fora da UI.
- Detalhes: `docs/foundation-005/F005-CLOUD-SECURITY-INCIDENT-001.md`.

## 5. C++ nativo escrito até este checkpoint

| Componente | Arquivo(s) | Evidência |
|---|---|---|
| Tipos transporte/CloudRoute e dispositivo | `libs/sentrium-device/include/sentrium/device.hpp` | integrado, U/D anteriores aprovados |
| Contrato túnel NetSDK e lifetime | `libs/sentrium-device/include/sentrium/cloud.hpp`, `cloud.cpp` | D-8 passou no Windows; **usa doubles e NÃO é conexão P2P** |
| Parser seguro DHP2P/1.0 | `cloud_wire.hpp`, `cloud_wire.cpp` | `D-9` criado, ainda sem resultado após último log |
| **NOVO:** transporte WinSock UDP nativo com endereço IPv4 numérico explícito, SO_RCVTIMEO, 1–3 tentativas, peer fixo, RAII, limites | `cloud_udp.hpp`, `cloud_udp.cpp` | código escrito; Windows build não executado após incremento |
| **NOVO:** composição UDP + parser com resposta 200/4xx e erros categorizados | `cloud_signaling.hpp`, `cloud_signaling.cpp` | código escrito; ainda não é bootstrap P2P |
| **NOVO:** CMake integra módulos + `ws2_32`, teste `D-10` offline | `CMakeLists.txt`, `tests/device/main.cpp` | aguardando execução |
| **NOVO:** validação em um comando | `scripts/windows/sentrium-validate-f005-cloud.ps1` | evita LNK1168 de processo aberto; configure/build + D9/D10 |

O novo código não contém credenciais ou servidores de bootstrap embutidos e **não faz requests ao serviço Cloud** por conta própria. Somente fornece camada de envio/recebimento para requisições **já autorizadas e assinadas** por futuro provider. Não apresentar isso como Cloud completo.

### Parte que falta para realmente conectar via Cloud

1. Contrato **legítimo** de autenticação/identidade do cliente Cloud e servidor bootstrap real (não copiar constantes/segredos de apps).
2. Negociação DH/P2P, rendezvous/NAT e fallback Relay compatíveis e comprovados; controle de timeout e autenticação.
3. Túnel TCP local dinâmico gerenciado pelo SENTRIUM.
4. Integração concreta `CloudTransport` → `CloudDeviceSession` → NetSDK → canais reais.
5. Teste efetivo com NVR Intelbras remoto de 16/32 canais, Direct/Relay e reconexão.
6. Avaliação de segurança/licença/serviço antes de distribuição.

**Situação REAL:** parser e transporte UDP criados; **Cloud remoto ainda NOT IMPLEMENTED/BLOCKED** por bootstrap/contrato. Não gastar tokens com testes artificiais ou declarar PASS sem dispositivo.

## 6. Histórico de falha LNK1168 — SUPERADA no checkpoint da seção 12

O operador anexou `Texto colado(20261026-164203).txt`: `git pull` até `34b337b...` PASS; PRE-FLIGHT PASS; CMake configure/generate PASS; fontes de `cloud_wire.cpp` e teste D-9 passaram pelo scan; no último passo, **LNK1168** ao linkar `sentrium-client.exe` porque o exe estava aberto/bloqueado para escrita. Não é erro conhecido do código C++ do parser. O teste D-9 **NÃO foi executado** no log. Não dizer `D-9 PASS`.

**Solução:** fechar normalmente a janela e processo `sentrium-client.exe`, depois executar o script novo, que impede link enquanto o cliente estiver ativo. Não encerrar processos do usuário automaticamente.

## 7. COMO VALIDAR EM UMA ÚNICA ENTREGA (PowerShell Developer)

No branch `foundation/005-device-connectivity`, após fechar SENTRIUM:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
.\scripts\windows\sentrium-validate-f005-cloud.ps1
```

O script já executa preflight, CMake configure/build e **apenas D-9 e D-10**. Para maximizar precisão, pedir **somente linhas de erro** se falhar (ou sumário PASS + HEAD), não milhares de linhas do /showIncludes. Não rodar benchmarks de F004.

## 8. Referências prioritárias para o próximo chat

- `docs/HANDOFF-SENTRIUM-CURRENT.md` — **esta fonte de verdade atualizada**.
- `CONTINUE-HERE.md` — ponte rápida para novo chat.
- `docs/foundation-005/SENTRIUM-CLOUD-P2P-DESCOBERTAS-CONSOLIDADAS.md` — evidência antiga de T2U e Relay; evitar extrapolações.
- `docs/foundation-005/F005-CLOUD-NATIVE-WIRE-001.md` — parser e primeiros contratos.
- `docs/foundation-005/F005-CLOUD-SECURITY-INCIDENT-001.md` — bloqueio Defender.
- `docs/foundation-005/SENTRIUM-CPP-Foundation-005-SCOPE.md` — backlog F005.
- `docs/foundation-005/F005-UI-SHELL-R1.md` — UX legado e estética adiada.
- `docs/HANDOFF-SENTRIUM-2026-10-25.md` — handoff histórico F004, **não usar como estado atual**.
- `docs/referencias-historicas/` e auditorias do XC Video IA — apenas referência, não dependência de runtime.

## 9. Regras operacionais

- Não inventar APIs, resultados PASS ou disponibilidade Cloud.
- Não mesclar `main` sem autorização, não aprovar Human Review/ADR por conta própria.
- Credenciais/Cloud ID/identificadores reais **nunca em Git/logs/prompts**; diagnósticos sanitizados.
- Não depender do executável suspeito de terceiros. Código próprio e auditável.
- Seguir metas do usuário: **implementar em lote, menos comandos, mais progresso, total rastreabilidade**.
- Não prometer entrega assíncrona; executar todo o desenvolvimento possível na interação.
- Atualizar este documento ao fim de cada bloco importante.

## 10. Mensagem ideal para iniciar a próxima conversa

> Continue o desenvolvimento **SENTRIUM VMS** a partir do repositório GitHub `servicosxc-xc/sentrium-vms`, branch `foundation/005-device-connectivity`. **Leia integralmente `CONTINUE-HERE.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` no GitHub e confira o HEAD real**. Não recomece fundamentos nem repita benchmarks. O Registro Automático já conectou um NVR real com 8 canais; Cloud P2P nativo está em desenvolvimento, sem SIM Next e sem executar `dh-fwd` (bloqueio Defender). O último build confirmado de 26/10 é **PASS [46/46]**, com **D-9 PASS e D-10 PASS**; histórico de LNK1168 e erro de PRE-FLIGHT foi superado. Já existe um script único de build/validação `scripts/windows/sentrium-validate-f005-cloud.ps1`. **Trabalhe em blocos grandes**, aplique código diretamente na branch, preserve `main`, atualize handoff e forneça comandos só ao fim.

## 11. Histórico de falha no PRE-FLIGHT — SUPERADA no checkpoint da seção 12

O operador atualizou a branch até `c7d7d08106d4b8a021d21785b77d757904a27fca` e executou `sentrium-validate-f005-cloud.ps1`. O `git pull` retornou `Already up to date`, mas o script **falhou antes do CMake/build** com:

```text
Where-Object : A propriedade 'DisplayName' não foi encontrada neste objeto.
scripts/windows/sentrium-preflight.ps1:57
PropertyNotFoundStrict
```

**Causa verificada no código:** o wrapper ativava `Set-StrictMode -Version Latest` e **dot-source** carregava o PRE-FLIGHT no mesmo escopo. O comando `Get-ItemProperty HKLM:...Uninstall\*` retorna itens sem `DisplayName`/`DisplayVersion`; acesso direto `$_.DisplayName` dispara `PropertyNotFoundStrict`.

**Correções commitadas:**
1. `sentrium-preflight.ps1`: consulta defensiva de `$_.PSObject.Properties['DisplayName']`/`['DisplayVersion']`, mantendo exigência original do nome e da versão do Windows SDK 10.1.26100.9457.
2. `sentrium-validate-f005-cloud.ps1`: muda a invocação de `. script.ps1` para `& script.ps1` e **adia `Set-StrictMode -Version Latest` para depois do PRE-FLIGHT**. A execução em escopo filho isoladamente não desliga a herança de StrictMode; a alteração da ordem é o que elimina a ativação precoce. As variáveis de ambiente `SENTRIUM_CMAKE`, etc. permanecem disponíveis no processo.
3. Identificação da saída de PRE-FLIGHT nesta branch corrigida para Foundation-005.

**Evidência:** reparo foi revisado no GitHub, **NÃO executado no Windows**. D-9/D-10 e compilação nativa seguem NOT RUN nessa tentativa. Não chamar de PASS até novo retorno do operador.

**Próximo comando único** (com SENTRIUM fechado):

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
.\scripts\windows\sentrium-validate-f005-cloud.ps1
```

**Modo de colaboração:** corrigir o problema real da etapa corrente sem iniciar novo ciclo de refatoração, builds repetidos ou testes sem valor; manter este documento atualizado a cada etapa relevante.

## 12. CHECKPOINT APROVADO PELO LOG REAL — DEBUG / D-9 / D-10 (26/10/2026 às 17:08)

O operador anexou a íntegra do PowerShell em `Texto colado(20261026-170803).txt`, referente ao **commit** `4185a7000940c2c31f8dbdd050198f5070429bf1` na branch `foundation/005-device-connectivity`.

**Evidência literal no final do log:**

```text
PREFLIGHT PASS: SENTRIUM Foundation-005 workspace
-- Configuring done
-- Generating done
[46/46] Linking CXX executable sentrium-client.exe
1/2 Test #28: D-9 .............................. Passed 0.03 sec
2/2 Test #29: D-10 ............................. Passed 0.01 sec
100% tests passed out of 2
F005 DEBUG BUILD + D-9/D-10: PASS
Commit: 4185a7000940c2c31f8dbdd050198f5070429bf1
Cloud bootstrap, Direct/Relay and real P2P login: NOT VERIFIED.
```

**Veredito:** build Debug + parser `D-9` e módulo UDP `D-10` **PASS** para esse commit. Os avisos `WrapVulkanHeaders` não foram impeditivos. A saída muito longa `Observação: incluindo arquivo` é varredura de dependências MSVC, não indicação de erro. A duplicidade da mensagem PRE-FLIGHT PASS também não prejudicou o resultado.

**IMPORTANTE:** testes D-9/D-10 usam dados offline; **não** estabeleceram P2P/Relay, sessão Cloud nem canal remoto. Não reinterpretar PASS como Cloud funcional e não exigir repetição de testes já aprovados sem alterações relacionadas.

**PRÓXIMA FRENTE DO PRODUTO:** conectividade remota real (bootstrap e autenticação Cloud com material legítimo), ou alternativa operacional independente usando o Registro Automático já comprovado, para acesso autorizado a NVRs remotos de 16/32 canais. Conservar o bloqueio de segurança do `dh-fwd`; nenhum SIM Next runtime. Avançar em blocos, com uma única validação por entrega, e manter `CONTINUE-HERE.md` atualizado se o estado mudar.

## 13. F005 R2 — PTCP e autenticação do dispositivo em código C++ (26/10/2026)

**Situação:** lote escrito diretamente na branch `foundation/005-device-connectivity`, **NÃO compilado no Windows após estes commits**. Não herdar PASS dos testes do commit `4185a700`.

### Entrega desta etapa

- `libs/sentrium-device/include/sentrium/cloud_ptcp.hpp` e `cloud_ptcp.cpp`: framing de datagramas PTCP em C++20 com header de 24 bytes, big-endian e tipos SYNC, PAYLOAD, BIND, STATUS, HEARTBEAT e comandos `0x17..0x1b`. Limites para payload (1280 bytes) e rejeição de dados malformados.
- `libs/sentrium-device/include/sentrium/cloud_auth.hpp` e `cloud_auth.cpp`: Windows CNG / BCrypt com **MD5 de interoperabilidade**, HMAC-SHA256, PBKDF2-HMAC-SHA256 (20 mil iterações) e transformação **AES-256-OFB** de endereços com IV fornecido pelo chamador. Usa apenas **credenciais que o operador possui do próprio dispositivo**, randsalt e nonce; não inclui nenhuma chave de aplicativo SIM Next, servidor fixo nem segredo de bootstrap.
- Estrutura `CloudDeviceAuth` não copiável, limpa memória de assinatura/chave derivada em destruição; entrada intermediária contendo senha também é zerada.
- `CMakeLists.txt`: adiciona PTCP/CNG ao `sentrium-device`, vincula `bcrypt`.
- `tests/device/main.cpp`: acrescenta `D-11` (framing PTCP) e `D-12` (assinatura/derivação, AES-OFB e entrada inválida).
- `scripts/windows/sentrium-validate-f005-cloud.ps1`: agora executa apenas `D-11/D-12`, **não repete D-9/D-10**; a saída de includes do compilador é gravada em `out/sentrium-debug/f005-cloud-build-latest.log` e reduzida no console.
- `docs/foundation-005/F005-CLOUD-PTCP-AUTH-R2.md`: especificação, limitações e instrução de validação.

### Limites explícitos

**Este código NÃO estabelece uma sessão P2P real.** Ainda faltam: bootstrap/autenticação da infraestrutura Cloud a partir de contrato legítimo; negociação do canal/rendezvous; sessões PTCP confiáveis e retransmissões/ACKs ordenados; Direct/Relay; ponte TCP local para NetSDK; teste efetivo com NVR remoto.

Dois níveis de credenciais são distintos: (1) **usuário/senha do NVR**, agora usados no módulo criptográfico CNG; (2) **identidade/autorização de aplicativo na infraestrutura Cloud**, ainda ausente e não deve ser extraída de SIM Next nem copiada de terceiros. O alerta Defender do `dh-fwd_win_x64.exe` mantém o piloto de terceiros desativado.

### Próximo comando único para o operador (após fechar SENTRIUM)

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
.\scripts\windows\sentrium-validate-f005-cloud.ps1
```

Essa execução fará build Debug e apenas testes D-11/D-12. **Ainda pendente de log Windows real.** Manter PR draft, `main` intacta, sem Human Review Approved.

**Fonte adicional da pesquisa de formato de protocolo:** <https://github.com/blackjid/dahua-p2p>, arquivos `ptcp/types.go` e `dh/crypto.go` (MIT). O C++ foi escrito independentemente; não foi integrado nenhum executável nem DLL externos.

## 14. Protocolo exato de retomada entre conversas — atualizado 26/10/2026

**OBJETIVO:** transferir o estado *reproduzível* do projeto e a intenção técnica sem depender da janela de contexto do ChatGPT. Todos os arquivos citados abaixo são versionados no GitHub. A conversa antiga pode atingir limite de mensagens sem afetar código, commits e documentação.

### Arquivos que o novo chat deve consultar ANTES de alterar código

1. `CONTINUE-HERE.md` (ponte curta com o prompt de início);
2. **este arquivo** `docs/HANDOFF-SENTRIUM-CURRENT.md` (histórico técnico vivo);
3. `docs/foundation-005/F005-CLOUD-PTCP-AUTH-R2.md` (último lote PTCP/CNG, não compilado após implementação);
4. `docs/foundation-005/SENTRIUM-CLOUD-P2P-DESCOBERTAS-CONSOLIDADAS.md` (comprovado vs inferido no estudo de engenharia reversa do legado);
5. `docs/foundation-005/F005-CLOUD-SECURITY-INCIDENT-001.md` (bloqueio obrigatório ao executável terceiros sinalizado pelo Defender);
6. `docs/foundation-005/F005-UI-SHELL-R1.md` (Home/abas e preferências ergonômicas; retoque estético adiado);
7. `docs/foundation-005/SENTRIUM-CPP-Foundation-005-SCOPE.md` e a PR #4 (escopo/governança);
8. `docs/HANDOFF-SENTRIUM-2026-10-25.md` apenas como retrospecto F004.

### Estado verificado / últimos checkpoints

- **GitHub** `servicosxc-xc/sentrium-vms`; branch de implementação `foundation/005-device-connectivity`; PR #4 **DRAFT**; **NÃO efetuar merge**, não aprovar ADR/Human Review sem ordem do proprietário.
- **Última execução Windows aprovada**: commit `4185a7000940c2c31f8dbdd050198f5070429bf1`, PRE-FLIGHT/CMake/Debug build **PASS [46/46]** e device `D-9/D-10 PASS` — **somente código R1**. Não atribuir PASS ao R2 ou às alterações de script feitas depois.
- **R2 mais recente (branch GitHub)**: `cloud_ptcp.*`, `cloud_auth.*`, BCrypt, CMake, D-11/D-12; **BUILD R2 / TESTES D-11/D-12 AINDA NÃO EXECUTADOS** no computador do proprietário.
- **Correção do runner**: `scripts/windows/sentrium-validate-f005-cloud.ps1` tinha `-R '^D-(11|12)` incompleto e código duplicado; reescrito corretamente no commit `3de0980fb5397503c06c5fe38cab509c75913676`, com `ctest -R '^D-(11|12)$'`. A correção só teve **inspeção de fonte**, não execução Windows.
- **AutoRegistro já comprovado no campo**: NVR Intelbras por `AutoRegistration` **Connected, 8 canais reais** (ID configurado no gravador coincide com o ID cadastrado; **não é serial físico**). Porta/IP de escuta acessível; falha de registro desconhecido em histórico separada da sessão conectada. Outros cenários/long-run ainda não comprovados.
- **Cloud operacional ainda não comprovado**: temos parser wire, UDP, sinalização, framing PTCP e autenticação do dispositivo (R2 código), porém faltam bootstrap legítimo da infraestrutura, P2P/Relay estável, túnel TCP local e NetSDK sobre Cloud real. Não afirmar que existe conexão Cloud funcional.
- **Dois níveis de autenticação diferentes**: credenciais de NVR próprias do operador (usadas nos módulos de interoperabilidade) ≠ credenciais/identidade de aplicativo exigidas pelo serviço Cloud (ainda não disponibilizadas). Não hardcodear/copiar segredos de SIM Next ou terceiros, nem inventar endpoints.
- **Incidente de segurança:** `dh-fwd_win_x64.exe` identificado por Windows Defender como `Trojan:Win32/Bearfoos.A!ml`, quarantena e execução bloqueada. Não restaurar, excluir Defender, executar ou recompilar para contornar. Cloud externo fica bloqueado tanto no QML quanto no DeviceController.
- **UI legado de referência:** XC Video IA r25, shell Home e abas estilo navegador, Visualização/Dispositivos/Eventos/Regras IA/Mídia/LPR/Configurações. O próximo ajuste dos botões Home: ícones pequenos quadrados arredondados com rótulo embaixo — **adiado em favor de Cloud**.
- **Modo de trabalho:** entrega C++/Qt em **lotes maiores**, sem Codex, atualizar documentação e GitHub, executar **um gate Windows focado no final**; não refazer 33 testes e benchmarks F004 sem causa; não exigir screenshots antigos novamente se estão nos documentos e no contexto.
- **Hardware/testes:** NVR LAN local limitado (não representa 16/32 câmeras); meta Cloud/AutoRegistro remoto para gravadores com 16/32 canais. F004 ingest NetSDK, PlaySDK e DHAV/FFmpeg demonstrados, mas não generalizar benchmark como aprovação 16/32.

### Próxima ação objetivamente correta

**Antes de continuar engenharia Cloud, obter UMA verificação focal R2 no Windows**, após fechar o SENTRIUM, porque esse lote adicionou CNG/PTCP e ainda não foi compilado:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
.\scripts\windows\sentrium-validate-f005-cloud.ps1
```

O script faz `git pull --ff-only` também, mas não há conflito em fazê-lo antes; pode invocar apenas o script após atualizar o repositório. Ele compila Debug e roda **somente D-11/D-12**. Se houver erro, ler código e log, corrigir na mesma branch. Não pedir credenciais Cloud/NetSDK ao usuário no chat.

Depois do gate, prosseguir num **lote substancial** rumo a negociação de sessão PTCP e transporte real, dentro da condição de autorização e segurança. Registrar commit validado, resultados reais, limitações e nova ação neste arquivo.

### Prompt que preserva intenção e reduz retrabalho

> Estou continuando o **SENTRIUM VMS** em um novo chat porque o anterior atingiu o limite. Utilize o conector GitHub: `servicosxc-xc/sentrium-vms`, branch `foundation/005-device-connectivity`. Leia integralmente `CONTINUE-HERE.md`, `docs/HANDOFF-SENTRIUM-CURRENT.md` e `docs/foundation-005/F005-CLOUD-PTCP-AUTH-R2.md`; confira HEAD real, docs relevantes e PR #4. **Não recomece projeto; não peça ZIP; não use Codex; não faça merge.** O AutoRegistro já conectou 8 canais reais. D-9/D-10 aprovados em Windows no commit 4185a700; **R2 PTCP/CNG e D-11/D-12 aguardam build**, runner corrigido. Cloud funcional ainda bloqueado por bootstrap/negociação; não depende do SIM Next e `dh-fwd` teve alerta Defender. Desenvolva em lotes maiores, priorize conexão remota real e preserve decisões, evidências e segurança. Corrija bugs no GitHub e dê só um comando focal de Windows ao fim. Atualize handoff ao concluir.

O próximo assistente **não precisa reproduzir toda a conversa antiga** nem possuir memória perfeita: precisa ler os artefatos citados e o HEAD do repositório, que são o registro técnico durável.


## 15. F005 R3 — PTCP session counter/ACK/retransmissão (26/10/2026)

Checkpoint iniciado sobre HEAD `42396b5d`; R2 PTCP/CNG D-11/D-12 ainda sem validação Windows. Criada classe C++20 `PtcpReliableSession` (`cloud_ptcp_session.hpp/.cpp`), com contadores de bytes, PID/LMID próprios, ACK cumulativo, detecção de duplicatas e lacunas de sequência e retransmissão com limite de 32 datagramas, 64 KiB, 400 ms e quatro tentativas. CMake e testes focalizados D-13/D-14 acrescentados; script único passa a rodar apenas **D-11 a D-14**, preservando o PASS histórico D-9/D-10. Documentação R3: `docs/foundation-005/F005-CLOUD-PTCP-SESSION-R3.md`.

**BUILD R2/R3 Windows: NOT RUN; D-11/D-12/D-13/D-14: NOT RUN.** Não interpretar inspeção estática como PASS. A camada não é provider Cloud: bootstrap autorizado/identidade de aplicativo, handshake, NAT/Direct/Relay, ordenação de stream/túnel TCP local e NetSDK Cloud real seguem bloqueados/não comprovados. Nenhum segredo, binário SIM Next, `dh-fwd` ou endpoint Cloud foi incorporado. Registro Automático de 8 canais segue PASS; teste remoto 16/32 canais ainda não executado. PR #4 draft; main e Human Review intactos.

**Validação focal única**, após fechar `sentrium-client.exe`, na pasta do repositório Windows: `\.\scripts\windows\sentrium-validate-f005-cloud.ps1` (ver script para pull --ff-only, PRE-FLIGHT, build Debug, D-11 a D-14). Ao receber log real, corrigir falhas na mesma branch sem repetir F004 ou D-9/D-10.


## 16. Reparacao do runner R3 apos ParserError real (26/10/2026)

Evidencia do operador: PowerShell detectou uma string sem terminador, chave de fechamento ausente e bloco try sem finally, ANTES de qualquer git pull, PRE-FLIGHT, build CMake ou CTest. Logo, R2/R3 D-11 a D-14 continuam NOT RUN.

Diagnostico conferido no GitHub HEAD f37ae9e4: o argumento ctest -R foi truncado (perdeu terminador da expressao regular e aspas) e o ultimo bloco foi duplicado apos o primeiro finally. A alteracao anterior usou substituicao textual JavaScript com sequencia especial cifrao+apostrofo na replacement string, que expandiu acidentalmente o sufixo inteiro do arquivo. Nao foi falha do SDK ou CMake.

Reparacao: arquivo PowerShell reescrito por completo. O regex de filtro agora esta completo para D-11, D-12, D-13 e D-14; existe um unico ctest, unico try/finally e bloco de encerramento. Feita verificacao estatica de conteudo, aspas/chaves balanceadas e ausencia de bloco duplicado. Execucao PowerShell nativa, compilacao e testes no Windows ainda NAO EXECUTADOS.

ATENCAO: o script local do operador esta invalido e nao consegue executar o pull embutido. E preciso executar o git pull --ff-only origin foundation/005-device-connectivity primeiro, depois executar scripts/windows/sentrium-validate-f005-cloud.ps1 com cliente SENTRIUM fechado. Nada foi alterado na main ou aprovado em Human Review; PR #4 permanece draft. D-9/D-10 seguem PASS anterior.


## 17. Checkpoint R2/R3 PASS no Windows e novo R4 TCP loopback (26/10/2026)

**Evidencia fornecida pelo operador:** PowerShell apos git pull ff-only ate o commit eb7317c9ce7b088d58cda2c3b883b3ced0dda8fa. PRE-FLIGHT PASS, CMake configure/generate PASS, Debug build [48/48] PASS, D-11 PASS (0.03s), D-12 PASS (0.03s), D-13 PASS (0.01s), D-14 PASS (0.01s), 4/4 PASS total 0.12s. Aviso WrapVulkanHeaders nao bloqueante. D-9/D-10 ja PASS no commit 4185a700. **R2/R3 COMPILADO E TESTADO (componentes offline); NAO significa Cloud remoto funcional.**

**R4 entregue como codigo, NAO compilado no Windows:** nova API CloudByteStream e CloudLoopbackBridge em libs/sentrium-device, implementacao nativa WinSock de listener de TCP 127.0.0.1 com porta efemera designada pelo sistema. Aceita um unico cliente local e encaminha bytes na ordem por um CloudByteStream entregue por futuro provider legitimamente autenticado. Sem binding externo, troca de firewall, Credencial Cloud, SDK de SIM Next ou executavel dh-fwd. Encerramento cooperativo de socket/thread e fechamento separado do NetSDK; uso so depois de provider real. Testes novos D-15 (rejeicao stream nulo, porta efemera, close idempotente) e D-16 (echo apenas em memoria via sockets reais loopback). CMake inclui arquivo e tests; script unico Windows **executa somente D-15/D-16** para nao repetir D-9 a D-14 aprovados. Documento: docs/foundation-005/F005-CLOUD-LOOPBACK-BRIDGE-R4.md.

**LIMITACAO:** esta ponte isolada nao implementa provider CloudTransport, autentica aplicativo, faz rendezvous/NAT, estabelece Direct/Relay, ou abre canais NetSDK de gravador real. Nenhum numero de canais remotos 16/32 comprovado. Manter Cloud UI bloqueado e rota Unknown. Nao chamar teste echo de tunel Cloud. PR #4 segue draft, main e Human Review intactos.

**Proximo gate focal (apos fechar SENTRIUM):** na pasta local Windows executar um unico script .\scripts\windows\sentrium-validate-f005-cloud.ps1. Script faz pull --ff-only, preflight, build e roda somente D-15/D-16. Este novo lote R4 esta NOT RUN ate evidencia do operador. Sem necessidade de refazer tests 9 a 14.


## 18. R4 Windows PASS e R5 bridge CloudTunnel ao NetSDK (26/10/2026)

EVIDENCIA REAL DO OPERADOR — commit c30d25c65b0331dac0006c000712453e83edc84b: git pull ff-only, PRE-FLIGHT PASS, CMake configure/generate PASS, DEBUG build [44/44] PASS; D-15 PASS 0,03 s; D-16 PASS 0,01 s; 2/2 PASS em 0,08 s. WrapVulkanHeaders nao bloqueou. D-9 a D-14 passaram nos checkpoints anteriores. D-15/16 medem ponte TCP loopback e eco de memoria, NAO Cloud remoto.

R5 IMPLEMENTADO NO GITHUB, WINDOWS NOT RUN: novo CloudStreamTunnel (cloud_stream_tunnel.hpp e .cpp) implementa o contrato CloudTunnel e entrega ao CloudDeviceSession uma porta TCP local efemera da CloudLoopbackBridge para um CloudByteStream fornecido por provider autenticado independente. Trata-se de encaixe de componentes, NAO implementacao de CloudTransport de producao ou prova de bootstrap. A rota reportada permanece Unknown; nenhum endpoint ou segredo de fabricante foi adicionado.

Correção de ciclo de vida em cloud.cpp: quando o NetSDK reporta falha ou excecao durante logout, a sessao e o tunel continuam ativos para tentativa posterior. O recurso so se encerra apos logout bem-sucedido; diagnosticos nao contem credenciais. D-17 usa FixtureStreamTransport e FixtureNetSdkSocket (SOCKET WinSock REAL para echo local), devolvendo artificialmente 16 e 32 canais apenas para testar a orquestracao, NAO evidencia de NVR real. D-18 exercita falha de logout, retentativa, proibicao de sessao duplicada, rota Unknown e invalidacao de porta apos close.

Script unico scripts/windows/sentrium-validate-f005-cloud.ps1 agora realiza build Debug + SOMENTE D-8, D-17 e D-18. D-8 foi incluído por mudanca em CloudDeviceSession::disconnect(), nao por repeticao de Foundations; nao repetir D-9..D-16 PASS sem necessidade. Documento R5 em docs/foundation-005/F005-CLOUD-STREAM-TUNNEL-R5.md. R5 BUILD/TESTES Windows ainda NOT RUN, sem reivindicacao PASS.

BLOQUEIOS REAIS CLOUD: identidade/autorizacao de aplicacao Cloud para bootstrap, resposta autenticada de rendezvous, negotiation Direct/Relay, reliable PTCP stream/reordering, camada CloudByteStream real e teste com NVR remoto 16/32 canais. Sem SDK Cloud do fabricante autorizado nao inventar hosts/tokens, copiar SIM Next ou usar dh-fwd retido pelo Defender. A Cloud UI permanece bloqueada. main inalterada, PR #4 draft e Human Review nao aprovado.

PRÓXIMO GATE: no PowerShell Developer, cliente SENTRIUM fechado, a partir do diretorio do repo, executar um unico comando:
.\scripts\windows\sentrium-validate-f005-cloud.ps1
Este script faz git pull --ff-only, PRE-FLIGHT, build e somente D-8/D-17/D-18. Informar log final ou trecho de erro. Nenhum dado sensivel no chat.


## 19. R5 PASS Windows e R6 recepcao de pacotes PTCP ordenada (26/10/2026)

**Log real recebido do operador para R5, commit 97159ea2c0e0a274f68f5e7f4066c194a4462a6a:** Git up-to-date; PRE-FLIGHT PASS; Qt 6.12.0, NetSDK, PlaySDK e baseline vcpkg validados; CMake configure/generate PASS; Debug build PASS conforme a linha final do runner; D-8 PASS 0,04 s, D-17 PASS 0,01 s, D-18 PASS 0,01 s; 3/3 testes PASS, total 0,10 s. Log de build foi resumido e NAO apresenta total [N/N] do R5; nao inventar esse valor. Avisos WrapVulkanHeaders nao impeditivos. D-9 ate D-16 PASS nos commits historicos e nao precisam ser repetidos. O R5 usou fixtures locais TCP e NAO acessou Cloud real.

**R6 implementado no GitHub, mas NOT RUN no Windows:** arquivos C++ cloud_ptcp_receive_window.hpp e cloud_ptcp_receive_window.cpp. PtCPReceiveWindow recebe pacotes de origem ja autenticada por um provider independente (AINDA AUSENTE), retendo ate 32 mensagens ou 256 KiB enquanto aguarda LMID contiguo. Control frames como Heartbeat avancam a ordem junto com Payload; trata rollover unsigned de LMID 32-bit, duplicatas, replay conflitante e erros de janela. Erro por gap excessivo/overflow/replay conflitante fecha e limpa a janela; pacotes fora de ordem nao sao entregues silenciosamente ao stream superior. A configuracao da primeira LMID deve vir de handshake comprovado, nunca de palpite. D-19/D-20 exercitam chegada invertida, duplicatas, intercalacao de controle/payload, rollover, conflito, gap e memoria. CMake registra os modulos e os testes; script unico Windows passou a executar APENAS D-19/D-20, sem repetir D-8...D-18 aprovados. Referencia: docs/foundation-005/F005-CLOUD-PTCP-REORDER-R6.md.

**Limites reais, ainda BLOQUEADO:** infraestrutura Cloud nao disponibilizou identidade/contrato bootstrap autorizado, nem rendezvous, autenticacao de peer, Direct/Relay medidos ou remontagem PTCP totalmente ligada ao CloudByteStream de producao. A janela nao envia ACK/retransmit e nao autentica; integrar sessao de rede e stream so apos obter protocolo/credenciais legitimamente autorizados. NVRs remotos 16/32 canais seguem NOT TESTED. Foco continua Cloud real, nao estetica. Sem SIM Next, dh-fwd em security hold. PR #4 draft, main preservada, sem Human Review.

**Proxima validacao focal (R6):** com SENTRIUM fechado, a partir do repo, executar um unico comando de pull ff-only e scripts/windows/sentrium-validate-f005-cloud.ps1 (agora somente D-19/D-20). Em nova janela Developer PowerShell, configurar novamente variaveis de ambiente Qt e SDK se nao existirem. R6 nao deve receber PASS ate evidencias Windows.


## 20. R6 MSVC fail and consolidated R7 post-auth data plane (26/10/2026)

OPERATOR REQUEST: increase size of coherent implementation batches. Complete offline C++ integration and coverage before requesting a single Windows build/debug; stop tiny compile cycles. Do not restart Foundations, approve Human Review or merge main.

R6 ACTUAL WINDOWS FAILURE at c52f2e98db6754d3258bb9697bd93c76406c2dad: PRE-FLIGHT PASS, CMake configure/generate PASS, Debug build FAIL with MSVC C4456 at tests/device/main.cpp line 789: for-loop variable id hides the id declared in main (line 217). With /WX warning promoted to C2220, build exit 2. D-19 and D-20 did NOT execute. The repeated showIncludes lines are build noise, not root cause. Fixed by renaming variable to messageIndex in the same file.

R7 IMPLEMENTED but NOT BUILT ON WINDOWS: cloud_ptcp_channel.hpp/.cpp implements a single post-auth PTCP data plane joining outgoing PID/LMID and cumulative ACK bookkeeping, <=1280-byte segmentation up to 16 KiB/write with atomic capacity precheck, LMID receive reorder across control and payload, explicit heartbeat, bounded original-wire retry and fail-closed on malformed frame, realm mismatch, future ACK, DISC or timeout. D-21..D-26 offline fixtures cover segmentation, reordering, replay, retry, remote disconnect, backpressure, ACK release and forged ACK. R6 D-19/D-20 are included in the SAME Windows build/test gate. CMake and one script updated to run ONLY D-19 through D-26, no repeat of historical D-8 to D-18.

ENV QUALITY: the focal runner detects known fixed Qt/Intelbras NetSDK/PlaySDK paths when unset in a fresh developer shell. It does not override explicit values, install software, change security settings or bypass the exact preflight.

R7 is NOT a working vendor P2P Cloud service. No legitimately authorized Cloud bootstrap application identity, rendezvous/peer authentication, verified Direct/Relay, live CloudByteStream or real 16/32 NVR has been supplied or field-tested. Do not invent endpoints or embed SIM Next or dh-fwd. Alternative under customer control: an independently secured site gateway/relay, which requires deployment and is not implemented.

Single Windows gate, with SENTRIUM closed and developer shell in repo: .\scripts\windows\sentrium-validate-f005-cloud.ps1. It performs pull ff-only, PRE-FLIGHT, Debug build, D-19..D-26. R5 remains the latest actual PASS. PR #4 draft, main intact. Details in docs/foundation-005/F005-CLOUD-DATAPLANE-R7.md.


## 21. R7 Windows PASS confirmado e R8 ponte PTCP byte stream (26/10/2026)

EVIDENCIA REAL DO OPERADOR: commit 2757ab6eab944b2c1953ad152532b0244e94c057; git pull ff-only OK, PRE-FLIGHT PASS, CMake configure/generate PASS, Debug build [44/44] PASS; testes D-19 ate D-26 **8/8 PASS**, 0,13 s. O warning de WrapVulkanHeaders nao impediu build. Nao repetir sem mudanca relacionada. Essa evidencia confirma dados PTCP sinteticos, NAO autenticacao Cloud real.

R8 IMPLEMENTADO DIRETAMENTE NO GITHUB, BUILD WINDOWS NOT RUN: nova API C++ PtcpVerifiedByteStream (cloud_ptcp_stream.hpp/.cpp) implementa CloudByteStream existente sobre VerifiedPtcpDatagramLink injetado por futuro provider autorizado. Integra R7 PtCPPostAuthChannel com fragmentacao, LMID ordenado, fila inbound de 128 KiB, relogio monotonic para retries, espera limitada por ACK com backpressure, deteccao de falha de envio parcial e encerramento cooperativo. Read/write em uma thread de worker, close pode ser acionado de outra; a implementacao do link externo precisa interromper receive/transmit ao fechar. A API chamada Verified nao garante por si propria autenticacao: requisito contratual a verificar por conexao real.

Novos D-27..D-32 incluem fixture de datagramas bidirecionais, reordenacao/replay, perda/partial transmit, ACK falso, desbloqueio da janela apos ACK e caminho REAL de socket TCP local (127.0.0.1) pela CloudLoopbackBridge ate datagramas de rede SIMULADOS e retorno. CMake inclui modulo R8, runner unico compila e executa APENAS D-27..D-32. Log de Windows ainda nao recebido deste novo lote. Documento tecnico: docs/foundation-005/F005-CLOUD-DATAGRAM-STREAM-R8.md.

BLOQUEIO PRINCIPAL PARA CLOUD POR SERIAL: identidade/SDK/bootstrap autorizado do provedor Intelbras; rendezvous validado, peer pinning, NAT Direct/Relay com telemetria, implementacao concreta de VerifiedPtcpDatagramLink e teste NetSDK com gravador remoto fisico 16/32 canais. Nao inserir endpoints inventados, segredos de app, DLLs SIM Next ou executar dh-fwd bloqueado pelo Defender. Se a operadora Cloud nao disponibilizar contrato/licenca, opcao de gateway remoto proprio exige novo escopo, infraestrutura e consentimento. Cloud UI nao deve ser apresentada como funcionando.

Main intacta, PR #4 draft e Human Review nao aprovado. Proximo gate unico: no Developer PowerShell no diretorio do repo, cliente SENTRIUM fechado, executar .\scripts\windows\sentrium-validate-f005-cloud.ps1; script faz pull ff-only, preflight e Debug build + D-27..D-32. R8 NOT RUN ate prova.


## 22. R8 hardening e gate consolidado (26/10/2026)

Antes do primeiro build Windows de R8, auditoria do novo PtcpVerifiedByteStream detectou risco de std::bad_alloc/excecao atravessando callbacks read/write noexcept e causando std::terminate. Foram acrescentadas barreiras catch-all fail-closed nos caminhos de retryDue(), progress() e write() para evitar derrubar todo o processo VMS em caso de falha inesperada de alocacao/estado. Os testes D-33/D-34 cobrem entradas invalidas (prazo, ponteiro, tamanho), escrita de tamanho excessivo e datagrama recebido acima da capacidade, sem leak ou continuação depois da falha. CMake e runner focal agora abrangem D-27 ate D-34 em um unico build. Nenhum teste R8 foi executado no Windows ate retorno do operador; log aprovado mais recente segue R7 [44/44] e 8/8 PASS. Os documentos anteriores com D-27..D-32 descrevem o escopo original; este checkpoint supersede. Cloud externo segue BLOCKED.


## 23. R8 single gate optimized to build only Cloud target (26/10/2026)

To reduce unnecessary Windows rebuild work, the single runner uses cmake --build --preset sentrium-debug --target sentrium-device-tests instead of rebuilding the entire SENTRIUM client/server/media lab. The sentrium-device-tests target transitively rebuilds and links all changed sentrium-device C++ sources, including R8, plus focused fixtures; CTest remains D-27..D-34. A PASS proves the native device/Cloud module, not fresh compilation of the entire Qt VMS client. Complete product integration remains a separate later gate. Exact PRE-FLIGHT and CMake configure are preserved; R8 NOT RUN until real Windows evidence.


## 24. R8 checkout atualizado, mas script antigo executou CTest R7 (26/10/2026)

EVIDENCIA REAL do log operador: antes do inicio a copia local tinha script R7 do commit 2757ab6. O pull dentro do script foi fast-forward ate 881085a29dfa2aae2d34fe94a5630f28b1bb4c7c; PRE-FLIGHT, CMake configure e Debug full build [44/44] PASS, mas o PowerShell executou D-19 a D-26 (8/8 PASS 0.12s) porque havia interpretado em memoria o runner R7 antes do Git atualizar o arquivo. Portanto D-27..D-34 de R8 NAO foram executados e permanecem NOT RUN. O full build de HEAD 881085a PASS nao e equivalente a testes R8.

CORRECAO permanente do runner no GitHub: ao iniciar captura blob SHA do proprio arquivo em HEAD, executa git pull ff-only, captura blob posterior; se o arquivo mudou, invoca nova copia do disco usando PSCommandPath e switch interno SkipPull para evitar segundo pull/build, e retorna da invocacao antiga. O novo runner revalida branch, worktree limpa e cliente fechado. Nao exige input adicional do usuario, nao altera main nem reduziu seguranca. O script que o usuario ja tem atualizado na copia local seleciona D-27..D-34, portanto na proxima execucao deve rodar testes R8 de verdade mesmo que o novo auto-reload so fique ativo em futuros pulls.

PROXIMO GATE UNICO: com SENTRIUM fechado, no diretorio do repositorio executar .\scripts\windows\sentrium-validate-f005-cloud.ps1; o script fara pull ff-only, build focal sentrium-device-tests e somente D-27..D-34. Ate log real, R8 testes = NOT RUN. Nao iniciar R9 ate esse gate; Cloud externo com bootstrap/Direct/Relay e NVR remoto 16/32 permanece NOT VERIFIED. PR #4 draft, main e Human Review intactos.


## 25. R8 PASS Windows confirmado e novo R9 UDP nativo (26/10/2026)

EVIDENCIA DO OPERADOR no commit 1294f5c482071e41cb6c0adb3676ef2c7c8ed94b: PRE-FLIGHT PASS, CMake configurado, Debug build FOCAL do sentrium-device-tests PASS, D-27 ate D-34 8/8 PASS (0,09 s). A saida nao certifica recompilacao integral do VMS, apenas modulo nativo e dependencias. R8 foi testado em modo simulado e nao conectou Cloud Intelbras. Nao repetir D-27..D-34 desnecessariamente.

R9 IMPLEMENTADO NO GITHUB, WINDOWS NOT RUN: cloud_udp_socket.hpp/.cpp, classe PinnedUdpSocket com WinSock UDP real. Endereco IPv4 UNICAST literal e porta fornecidos explicitamente por um futuro provider autorizado, sem DNS nem endereco embutido. Possui WSAStartup/cleanup, socket exclusivo, bind local com porta efemera, UDP connect para fixar endereco do peer no kernel, limite de datagrama 2048 bytes, select em intervalos maximos 20ms e deadline 0..1000ms, read/send inteiros ou erro, close concorrente/idempotente; fechamento seguro em datagrama truncado, erro de socket ou origem inadequada. UDP connected e apenas filtro IP:porta, NAO autenticacao de peer. A classe NAO implementa VerifiedPtcpDatagramLink, impedindo uso acidental como transporte P2P autenticado.

Testes D-35..D-40 usam sockets UDP Windows REAIS em 127.0.0.1: valida endereco, transmissao bidirecional, fonte nao autorizada filtrada, truncamento, close simultaneo e duas sessoes independentes. Nao testam gravador fisico nem servidor Intelbras. CMake integra novo .cpp e registros; runner unico agora compila SOMENTE sentrium-device-tests e seleciona D-35..D-40. R9 build e testes ainda NAO EXECUTADOS no Windows.

BLOQUEIOS REAIS: identidade/aprovacao oficial do bootstrap Intelbras Cloud, resposta autenticada do rendezvous, peer verificado, NAT Direct/Relay e prova de rota, promocao segura para VerifiedPtcpDatagramLink, teste remoto fisico 16/32 canais pelo NetSDK. Sem SIM Next nem dh-fwd do Defender. PR #4 draft, main intacta e Human Review nao aprovado. Proximo comando focal unico: script windows sentrium-validate-f005-cloud.ps1 (cliente fechado, repo root). Ver documento R9.


## 26. R9 PASS Windows e R10 integração real TCP/PTCP/UDP de laboratório (27/10/2026)

**EVIDÊNCIA REAL DO OPERADOR — R9**: Developer PowerShell atualizou branch foundation/005-device-connectivity de 1294f5c para 0f639c3e9638dcbd17850c0b118869032945fc2b. Auto-reload do runner funcionou (mensagem `Runner changed on Git pull: loading updated script.`). PRE-FLIGHT PASS 2x, CMake configure/generate PASS, compilação focal `sentrium-device-tests` PASS com link [10/10], D-35 a D-40 **6/6 PASS (0,26 s)**. Avisos WrapVulkanHeaders e /showIncludes não foram falhas. Não repetir R9 sem modificação relacionada. Esse PASS prova transporte UDP Windows e isolamento de portas locais, NÃO P2P Intelbras.

**R10 EM GITHUB, WINDOWS NOT RUN**: novo executável **exclusivamente de teste** `tests/device/cloud_network_lab.cpp` (sem alterar biblioteca de produção). Um respondedor de laboratório via SOCKET UDP real 127.0.0.1 recebe datagramas PTCP, ecoa corpo com LMID/ACK sintéticos, permite perda de primeiro pacote, ACK forjado, realm inesperado, injeção de origem diferente e silêncio. Adapter `UnverifiedLabLink` envolve `PinnedUdpSocket` R9 e implementa a interface R8 SOMENTE DENTRO DO EXECUTÁVEL DE TESTE. Ele NÃO autentica peer; sua presença não habilita a função Cloud nem serve de provedor em produção.

Testes integrados:
- **D-41**: fluxo PTCP de 3KB segmentado em UDP real com retorno ordenado R8+R9.
- **D-42**: socket TCP real no loopback → CloudLoopbackBridge R4 → PtcpVerifiedByteStream R8 → UDP real R9 → respondedor local → retorno completo TCP (sem NetSDK real).
- **D-43**: datagrama de outra porta de origem rejeitado pelo filtro UDP conectado, sem impedir resposta do peer fixado.
- **D-44**: realm inválido e ACK futuro forjado por peer simulado geram encerramento fail-closed.
- **D-45**: perda intencional do primeiro UDP e retransmissão PTCP original recuperam os dados.
- **D-46**: `close()` simultâneo cancela leitura UDP, impede nova escrita e finaliza cooperativamente.

CMake adiciona apenas o alvo `sentrium-cloud-network-tests` ligado a `sentrium-device`, com /W4 /WX, e registra D-41 a D-46. Runner único atualiza/recarrega script, PRE-FLIGHT, configura Debug e compila SOMENTE esse novo alvo e dependências; executa SOMENTE D-41 a D-46. Não reexecutar D-9 a D-40, já aprovados. Os testes usam sistema operacional REAL no localhost, mas **nenhum servidor ou NVR remoto, nenhuma autenticação Cloud, nenhum login NetSDK**. Um nome de interface contendo Verified não significa que a fixture de teste autentique: o adaptador de laboratório é explicitamente INSEGURO para produção.

**Próxima dependência de produção**: obter SDK/contrato de aplicativo Cloud oficialmente autorizado (bootstrap, rendezvous, prova criptográfica de peer e Direct/Relay) OU autorizar um gateway remoto gerenciado próprio. Antes disso, `VerifiedPtcpDatagramLink` NÃO deve ser concretizado com simples `PinnedUdpSocket` e exposto aos clientes. Cloud por serial e NVR remoto 16/32 permanecem BLOQUEADOS. Sem copiar SIM Next, sem dh-fwd bloqueado pelo Defender, sem endpoints/segredos inventados. PR #4 permanece draft, main intacta, Human Review não aprovado.

**PRÓXIMO ÚNICO GATE**: no Developer PowerShell, na pasta do projeto e com SENTRIUM fechado, executar `\.\scripts\windows\sentrium-validate-f005-cloud.ps1`. O autocarregamento do script cuida do pull; D-41..D-46 ficam NOT RUN até receber evidência Windows. Após esse teste, evitar novos lotes de simulação e priorizar autorização real do provider.


## 27. R10 Windows PASS e dependência externa Cloud confirmada — 27/10/2026

Log real no commit c211b964eefea6caa9f74c88bba9095ef9984e6d: PRE-FLIGHT PASS, CMake configure/generate PASS, build sentrium-cloud-network-tests [8/8] PASS, D-41..D-46 6/6 PASS em 1,38s. Runner auto-reload OK. Testes TCP+PTCP+UDP reais SOMENTE localhost com peer sintético; não Cloud Intelbras remoto. Não repetir tests aprovados.

Pesquisa pública de documentação Intelbras identificou procedimento de SDK/API para DVR/NVR, mas resposta de 2018 distingue SDK IP/DDNS de Cloud P2P serial e dizia não disponível naquela época; estado 2026 INDETERMINADO. Portal Moovsec Developers existe mas equivalência com Cloud serial NVR NÃO COMPROVADA. Documento novo docs/foundation-005/F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md contém solicitação ao fabricante, referências e critérios de habilitação. Próxima etapa de desenvolvimento depende de SDK/contrato autorizado ou escolha explícita de arquitetura de gateway remoto seguro. Não implementar Cloud provider fictício, não usar SIM Next/dh-fwd, não declarar conectado. PR #4 draft, main e Human Review intocados.


## 28. RE-001 — Plano de captura passiva SIM Next e gate de interoperabilidade (27/10/2026)

Decisao do usuario: explorar engenharia de interoperabilidade do SIM Next com dispositivos proprios/autorizados para descobrir viabilidade do Cloud por serial antes do contrato/licenciamento formal. Isso nao autoriza copiar identidades/segredos de app nem burlar controles ou distribuir software de terceiro.

Novo script SOMENTE LEITURA scripts/windows/sentrium-observe-simnext.ps1 detecta processos SIM Next/filhos, nomes de bibliotecas carregadas (sem binarios), assinatura/hash/versao do executavel, conexoes TCP com destino pseudonimizado por salt aleatorio local, portas locais UDP e estado/timing. Reporta JSON em out/simnext-interop (ignored pelo Git), sem payload, memoria, senhas, seriais, TLS keys ou captura automatica. Get-NetUDPEndpoint nao mostra peer UDP; uma captura Wireshark em rede propria sera etapa separada e revisada, sem envio de PCAP bruto. Documento docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001.md contem sequencia Connecting/Idle/Live/Relay (se realmente marcado na UI), perguntas de viabilidade e stop/go. A rota Direct/Relay vem da UI oficial SIM Next; nao inferir pelos pacotes.

Estado atual: script com commit GitHub, **NAO TESTADO no Windows**, nenhum dado SIM Next coletado ou acesso Cloud comprovado. Proximo comando unico na pasta repo, com SIM Next instalado e aberto: .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Connecting . Ele pede ENTER antes de observar, quando usuario deve clicar na conexao Cloud autorizada. Se PIDs nao encontrados, consultar lista de processos e repetir com -ProcessIds. Nao rerodar D-41..D-46 R10 ja PASS. Manter main/PR draft/Human Review sem alteracoes, sem SIM Next DLLs, dh-fwd ou seguranca relaxada.


## 29. RE-001 first real SIM Next observation analyzed (27/10/2026)

User attached actual Connecting JSON and local salt. From the JSON: SIMNext.exe 1.25.4.1203, pid 22076, valid signature, 196 module names, 25.2 s; 411 grouped rows = 322 TCP + 89 UDP source ports. TCP: 161 unspecified IPv4, 1 unspecified IPv6, 146 loopback, only 14 nonlocal remote state rows; 144 loopback Established = 72 bidirectional pairs. 37000 TCP 3 SynSent attempts; 37777 one preexisting Established and 2 fresh SynSent; 37001/37719/443 also observed. Do not infer Cloud peer or NVR from service ports; Get-NetUDPEndpoint has no remote address. Need operator UI status and Idle-vs-Live experiment. Sanitized analysis in docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-01.md; never upload raw salt/JSON/payload to GitHub. Salt was provided so privacy tagging is no longer private; suggest rotation for future shared observations (but breaks comparison continuity). No production code or build change. R10 6/6 Windows PASS retained; PR #4 draft, main and Human Review untouched.


## 30. RE-001 OBS-02 — Connecting versus Idle on real Windows captures (27/10/2026)

Operator uploaded Idle JSON + redundant Connecting JSON + local hash salt. Offline differential verified same SIMNext.exe v1.25.4.1203, PID 22076, same executable hash. Connecting 25.2s TCP322 UDP89; Idle 26.08s TCP500 UDP112. Local/wildcard TCP 308->470, remote TCP 14->30, SynSent 5->20; localhost Established reciprocal pairs 72->91. Remote TCP ports: 37000 unsuccessful SynSent 3->8; 37777 same single Established plus SynSent 2->8; 37002 SynSent 0->4; TCP 7000/12246/37001/37719/37777 established sockets persisted BOTH snapshots. UDP local ports overlap 72, 17 only Connecting and 40 only Idle. Idle firstSeen ~13.415s had 70 grouped entries (incl 22 UDP), showing strong background churn with no video tile necessarily open. No P2P route can be inferred, no UDP destination. No packet payloads or count/size; user action/UI state for Connecting remains unknown. SANITIZED report docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-02.md; never include raw JSON, salt, tag values or user secrets in repo. Next ask user to collect Live with ONE camera after ENTER and record P2P/Relay/IP UI status. Need no C++ build; R10 6/6 PASS retained, main and Human Review unchanged, PR draft.


## 31. RE-001 OBS-03 — Third real Live capture and RE-002 Wireshark next gate (27/10/2026)

Analyzed operator-supplied 25.99s Live JSON against previous Idle and Connecting with the same PID/version. Live grouped TCP 446, UDP local104, local TCP426, offhost TCP20, SynSent11, localhost Established 91 reciprocal pairs. Idle TCP500, UDP112, local470, offhost30, SynSent20, pairs91; Connecting TCP322 UDP89, local308, offhost14, SynSent5, pairs72. 535/550 Live entries first observed at t=0 and ALL 104 UDP endpoints present at t=0; nine off-host TCP Established were present at t=0 AND in Idle: 7000,12246,15832,28295,37001,37719,37777,44431,57314. No new off-host TCP Established after initial sample; does NOT disprove UDP Cloud. SynSent on 37000/37002/37777 persisted in both Idle and Live, cannot label P2P/Relay. No UI actual video success or route yet reported. Sanitized report docs/foundation-005/F005-SIMNEXT-INTEROP-RE-001-OBS-03.md; NO raw data/salt copied. Next one targeted operator Wireshark capture with 10s Idle, 20s camera EXTRA streaming, 10s closed; focus UDP conversations byte growth. Keep PCAP private, share only sanitized statistics. No C++ build or test rerun, R10 6/6 PASS. PR #4 draft, main and Human Review unchanged.


## 32. RE-002 OBS-01 — Wireshark UDP media candidate in real export (27/10/26)

Operator sent screenshot of Statistics > Conversations with TCP TAB selected (33 TCP conversations; UI indicated UDP 234) and a long exported packet list. Local parsing read 4707 lines, capture time 0..13.157s: UDP4424 rows / 2,369,454 bytes; TCP233 lines; rest DNS/TLS/etc. Top UDP conversation by far: remote port1639 vs local port64220, 1737 packets 2,007,020 bytes; inbound1546 packets 1,994,186 B, outbound191 12,834 B, 84.7% of UDP bytes. Traffic was <1100B/s in the first four seconds, then sustained 206–239 kB/s from t~4.656s through sample end, with 1197 datagrams at 1442 bytes, 151 at 946 bytes. ~1.8 Mb/s inbound consistent with stream. Port pair only, **NO remote private/customer IPs nor salted tags** committed. Some small outbound messages at t4.12 and t4.24 preceded burst but do not prove Cloud bootstrap/auth. The user has NOT explicitly confirmed image opened, UI P2P/Relay or IP route. The export is ~13.16s, does not include planned stream close, so hypotheses remain provisional. Detailed report docs/foundation-005/F005-SIMNEXT-INTEROP-RE-002-OBS-01.md. Next one bounded 40s capture 10s idle+20s video+10s tile closed, UDP Conversations tab (not TCP), local display filter udp.port == 1639 && udp.port == 64220, screenshots with IP redacted, IO graphs and UI route. No C++ updates/build, R10 Windows PASS 6/6; PR draft, main and Human Review remain untouched.


## 33. RE-002 OBS-02 — 49s Wireshark full PCAPNG; proprietary UDP header observed (27/10/26)

Actual user uploaded full PCAPNG ~7.2MiB; offline Python parsed 15,087 Ethernet frames and 48.957s. Dominant bidirectional UDP stream (remote port1639, local64220) = 4508 frames, 5,484,597B wire; 4068 incoming frames 5,456,667B and 440 outgoing 27,930B. First large (>=900B) inbound packet t13.913s, last t36.482s. ~1.92Mbps while t14..36, then no further media-size datagrams; compatible with opened/closed camera in operator RE-002 schedule but still no explicit UI P2P/Relay/IP or video success report. **Major new protocol evidence**: UDP envelope leading types 12 01 10 00/01/02/03 observed across 1725/1710/6481/2641 packets. ALL 6481 12 01 10 02 packets have big-endian offset 8..9 length exactly UDP payload size -16 (16-byte envelope). In main flow 4035 received type-02 sequential IDs offset4..7 were unique and contiguous, zero observed sequence gaps, and 405 outbound type-03 ACK-like datagrams at offset8..11 refer to those IDs. Do NOT claim authenticated peer; capture host ACK latency is not RTT. No ASCII PTCP present in any captured UDP payload; existing C++ cloud_ptcp.cpp expects literal PTCP 24-byte framing. THEY ARE NOT INTERCHANGEABLE ON WIRE without another proven transformation/layer. Need properly map vendor bootstrap/rendezvous/identity and confirm remote IP owner/route; Direct vs Relay unresolved. Sanitized doc docs/foundation-005/F005-SIMNEXT-INTEROP-RE-002-OBS-02.md. No raw PCAP, IPs, app secrets, serials or payloads in GitHub. No product code changed, R10 Windows 6/6 PASS unchanged, main intact, PR #4 draft, F004 Human Review pending.


## 34. RE-002 OBS-02 operador confirmou P2P e abrir/fechar canal — RE-003 bootstrap (27/10/26)

Usuario confirmou expressamente: iniciou captura Wireshark SEM camera, depois abriu um canal e por fim fechou; SIM Next indicou **P2P**. Agora a associacao com fluxo UDP de video (porta remota 1639 ↔ local 64220; 4508 pacotes 5.48MB e janela t~13.913 a t~36.482s) e sustentada tambem pelo teste de campo. NAO significa que remoto e o proprio NVR ou ausencia de relay; nao prova app Cloud independente ou qualidade da imagem (nao mencionada explicitamente). Header observado 12 01 10 xx/16B segue distinto de PTCP ASCII/24B implementado experimentalmente. Atualizacao em docs/foundation-005/F005-SIMNEXT-INTEROP-RE-002-OBS-02.md. Proximo gate RE-003: capturar inicio SIM Next (fecha normalmente, comeca Wireshark antes de abrir, registra online / camera extra / status P2P), em gravador autorizado, sem remover dispositivos; analisar apenas fluxo/timing e deixar PCAP privado. Nenhuma mudanca C++ ou novo build. R10 Windows 6/6 PASS, PR draft/main/Human Review preservados.


## 35. RE-003 OBS-01 — Real recorder WAN IP corroborates Direct-to-site P2P in 2 PCAPs (28/10/2026)

User supplied current WAN public IP of site housing DVR. Re-analyzed both native PCAPNG: exact same WAN IPv4 is remote endpoint of RE-002 stream (UDP remote1639/local64220 4508 frames 5,484,597 wire B) AND RE-003 startup stream (UDP remote1131/local51699 3964 frames 3,586,029 wire B, plus 1 UDP probe remote1132). UI P2P confirmed by user. **High confidence that observed media is site-direct P2P to DVR network's WAN IP, rather than flowing via an external vendor relay**; DOES NOT prove endpoint is DVR not NAT/router, or that Intelbras discovery servers do not participate in session authorization. Repo never contains literal WAN IP or original PCAP/packets/tokens. RE-003 5135 frames 73.320s; DNS Intelbras-named queries at t11-14; first site UDP t40.767, bidirectional port1131 from t41.369; handshake-like header family 12 20 D0 07 / D1 07 / D2 07 near t41, then 12 01 10 xx. Media bulk ~t56-70. No claim exact auth semantics; no independent Cloud app permission. Sanitized report docs/foundation-005/F005-SIMNEXT-INTEROP-RE-003-OBS-01.md. Existing experimental PTCP/24B not wire-compatible with observed 16B 12 01 10 xx. No C++/CTest changes, no build; R10 6/6 PASS, PR #4 draft, main/F004 Human Review intact.


## 36. RE-004 — Reproducible offline startup/handshake audit, 28/10/2026

From user's existing Wireshark RE-002 and RE-003 PCAPNG, wrote **scripts/research/sentrium-p2p-pcap-audit.py** (Python stdlib only, read-only, no packet payload/IP/token output), **tests/research/test_re004_pcap_audit.py** (4 synthetic unit tests) and **scripts/windows/sentrium-audit-f005-pcap.ps1** (single-command wrapper, Windows NOT RUN). Actual offline runs: tests 4/4 PASS; RE-002 15,087 packets/48.957s dominant UDP remote1639/local64220 4508 frames/5,484,597B, type02 length 6481/6481 PASS; RE-003 5135 packets / **73.689s (rechecked; older doc said 73.320)**, dominant remote1131/local51699 3964 frames/3,586,029B, precursor remote1132 t40.818, type02 length 3254/3254 PASS, sustained-video t56. DNS answer->TCP443 SYN matched api-cronos at t11.260, api.simnext.isec at t12.662, logs.isec at t14.429; these contacts do NOT prove auth. Site UDP handshake-like sequence t41.420 D0 07 -> t41.494 D0 and D1 -> t41.520 D2; second D0/D1/D2 t42.055-42.078; 12 01 10 00/01 t42.084-42.110, 05/06 t42.125-42.155, 02/03 ~t42.175+. Only lengths, headers/direction/timing are reported, not payload. Sanitized documentation **docs/foundation-005/F005-SIMNEXT-INTEROP-RE-004-METADATA-AUDIT.md**. Original PCAP never committed. Cloud provider authorization/authentication still BLOCKED, DO NOT equate captured packets to credentials or C++ PTCP. No changes to product C++/CMake; R10 Windows 6/6 PASS stands; PR #4 draft, main/F004 Human Review intact. Next gate: official app identity/SDK and supported bootstrap; Windows wrapper can be tested by operator with an existing PCAP only, no new capture necessary.


## 37. RE-004 Windows Python launcher resolution hotfix (28/10/2026)

User executed RE-004 on Windows, Python launcher `py -3` tried deleted `C:\Users\ericx\AppData\Local\Programs\Python\Python313\python.exe`, error Unable to create process, wrapper correctly threw without declaring PASS. The file path is confirmed valid at `C:\Users\ericx\Downloads\wireshaark 2.pcapng`, do not ask path again. FIX wrapper performs real Python 3.9+ execution probe using -c and exit status before choosing, fallback `python`/`python3`, explicit `py -3.12` etc, `%LOCALAPPDATA%\Programs\Python\Python*\python.exe`, optional `-PythonPath`. If none present fails with diagnostic/reinstall guidance, no false PASS; no pip required. Need Windows operator retest to validate wrapper; Python unittest 4/4 PASS from prior non-Windows context does not validate PowerShell. Docs RE004 updated; C++ R10 6/6 PASS remains, PR #4 draft and main/F004 Human Review unchanged. Do not expose actual WAN IP or PCAP/secret artifacts.


## 39. RE-004 validation with TShark, automatic Python-free partial fallback (28/10/26)

Operator ran installed TShark on RE-003 PCAP under Windows, count3964 bytes3586029 for udp.port==1131 matching offline Python. Python launcher stale and no working Python. Windows RE-004 wrapper now auto-falls back to in-memory TShark UDP IPv4 flow-size grouping, output sanitized JSON only (no IP/MAC/serial/packet data), with distinct schema sentrium.f005.pcap.tshark-transport.v1 and PASS_PARTIAL_TRANSPORT_ONLY. This is NOT full Python RE-004 audit; no DNS, envelope or auth. The new TShark fallback was not run on Windows yet; user should git pull and rerun same PCAP command. Previous Python tests4/4 PASS in dev; C++ R10 Windows 6/6 still PASS. PR4 draft/main/F004 Human Review preserved.

## 40. RE-004 Windows PASS PARCIAL comprovado; F005 Cloud SDK gate permanece (28/10/2026)

Operador executou apos git pull ed67873f o script `sentrium-audit-f005-pcap.ps1` com arquivo RE-003 local, recebeu `RE-004 TSHARK PASS PARCIAL: 3964 pacotes / 3586029 bytes na maior conversa UDP.` Relatorio JSON gerado em out/simnext-interop/re004-tshark-transport-20261028-041303-716.json. Log prova resultado de contagem byte/pacote + criacao do relatorio, mas NAO inspecionamos o conteudo do JSON no Windows. Auditoria Python DNS/framing Cloud nao executada; portanto PASS PARCIAL e nao PASS completo. Nenhuma instalacao Python ou recaptura/novo build necessario. C++ R10 Windows D-41..D-46 6/6 PASS mantido.

Pesquisa de fontes oficiais (out/2026): Intelbras forum 03/07/2018 diferencia Cloud SDK por serial de NETSDK IP/DDNS e afirma indisponibilidade naquela data; orientacao oficial 16/10/2024 encaminha API/SDK via chatbot suporte mas NAO confirma Cloud; Guardian Parceiro manual v1.6.0 explica conexao P2P e fallback Relay mas e um produto oficial, nao SDK publico. Endpoints/docs links anexados a F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md. Gate externo: solicitar oficialmente SDK Cloud por serial/identidade app/licenca Direct/Relay e condicoes de terceiros. Sem contato externo enviado; decisao do operador. Branch foundation/005-device-connectivity PR4 draft, main e F004 Human Review intocados.


## 41. User decision: Cloud SDK vendor not obtainable via chatbot (28/10/2026)

User reports Intelbras support only chatbot, SDK/API menu offers recorder NetSDK (already supplied), not specific Cloud/SIM Next SDK. Do not repeat request as blocking next step. Created docs/foundation-005/F005-NO-VENDOR-CLOUD-SDK-PLAN.md with actionable alternative: (A) existing, real 8-channel NVR validated AutoRegistration via official NetSDK CLIENT_ListenServer and LoginEx2 SERVER_CONN, needs secure reachable listener for remote/CGNAT use and callback/reconnect hardening; (B) VPN WireGuard + standard LAN NetSDK where customer router/gateway can be managed; (C) Intelbras Cloud-by-serial protocol research remains passive and BLOCKED for independent authenticated bootstrap, not product. Non-equivalence of AutoReg and Cloud serial explicit; user goal kept intact. No need new chatbot contact; no cloud credential extraction. No source code/build change, R10 Windows D-41..46 6/6 PASS; PR4 draft, main and F004 Human Review intact.


## 42. Cloud library LAB-001 — isolated feasibility experiment by operator request (28/10/2026)

User explicitly requested exploring loading original SIM Next Cloud DLLs into an independent test harness, with vendor legalization/redistribution request after technical validation. Old project reports successful Relay tunnel via VVEyeAdapter(null,null)/AddSDKPort and local TCP bridge under .NET 10, but only with original installed components; not a Windows-clean distribution or legal commercial permission. Branch now adds tools/simnext-cloud-lab C#/.NET 10 x64 harness and scripts/windows/sentrium-simnext-cloud-lab.ps1. INSPECT mode (default) checks local binaries/meta/signatures without loading vendor assemblies or contacting cloud. Experimental Connect mode explicitly prompts permission, Cloud UUID and remote service port locally, uses assembly resolver and public VVEyeAdapter APIs, checks P2pIsActive/RelayConnection/bind scope, cleans up; **do NOT recommend running until inspection + rights evaluated**. No vendor binary copied/redistributed, no app secret discovery, no SIM Next installation alteration, no extra dependencies or runtime product integration. Code NOT compiled or run in Windows. Main/cloud.cpp unchanged. Strictly mark lab statuses NOT RUN, current native R10 6/6 synthetic test baseline unaffected; PR4 draft, F004 Human Review preserved. Doc docs/foundation-005/F005-SIMNEXT-CLOUD-LIB-LAB-001.md. Next one command Windows `git pull` + `.\scripts\windows\sentrium-simnext-cloud-lab.ps1 -Mode Inspect` and choose vendor DLL original.


## 43. F005 LAB-001 INSPECT PASS on operator Windows (28/10/2026)

Operator successfully executed git pull to 0b0cdf1 then `sentrium-simnext-cloud-lab.ps1 -Mode Inspect`. Tool compiled and ran on Win/VS2022; all five vendor DLL files in original local directory. Signatures Valid SIMNext.P2PSDK/Core/Model; NotSigned Libt2uWrapper.dll/libt2u.dll. Metadata PE machine AMD64 for P2PSDK/Core/Model/libt2u; Libt2uWrapper managed I386 (possibly AnyCPU IL; not yet proven 32bit-required). INSPECT PASS, no DLL loaded or network connection done. The next step is LAB-CONNECT only with operator authorization and when lawful/license terms permit, no bundle/redistribute vendor files and no credential extraction. Script requests interactive confirmation, Cloud UUID locally and actual device service port, attempts vendor AddSDKPort then P2pIsActive and RelayConnection, checks TCP bind and cleans up. No actual LAB CONNECT Windows test yet; no NetSDK login or video proven by this new harness. Product C++ R10 6/6 synthetic lab historical, main/F004 Human Review preserved, PR4 draft. Documentation docs/foundation-005/F005-SIMNEXT-CLOUD-LIB-LAB-001.md updated.


## 44. LAB-001 CONNECT first real Windows result: vendor tunnel ACTIVE, unsafe TCP bind gate (28/10/2026)

User executed lab -Mode Connect with owner/authorized recorder. Locally installed SIM Next DLLs loaded successfully; `ADAPTER_READY`, `TUNNEL PORT_RETURNED`, `TUNNEL STATE ACTIVE`, `TUNNEL ROUTE NOT_RELAY_REPORTED`, `LOCAL TCP LISTENER OBSERVED`. `BIND SCOPE NOT_PROVEN_LOOPBACK` triggered explicit SAFETY STOP exit9, so no NetSDK login/video. Important: no `Direct` claim because false RelayConnection is not proof; no proof of authenticated independent third party cloud or redistribution. Prior attempt did echo a Cloud UUID into chat text; NEVER reprint or commit this private ID. Fixed C# console input to hide Cloud UUID with ReadKey(intercept:true), and added sanitized listener-class diagnostics LOOPBACK/WILDCARD_IPV4/WILDCARD_IPV6/SPECIFIC_NON_LOOPBACK_IPV4/v6 (no addresses). No firewall changes, no NetSDK login, no DLL copy. Existing safety gate still stops all non-loopback listeners; Windows listener table lacks PID attribution. Commit new C# change needs BUILD Windows/test with 1 command run original lab after git pull; if wildcard confirmed, isolate securely before production. R10 native synthetic 6/6 PASS unchanged; PR4 draft and main/F004 Human Review preserved.


## 45. LAB-001 second Cloud connect exit=7; added safe R2 diagnostic (28/10/2026)

Owner operator's next Windows lab returned `ADAPTER_READY` then `TUNNEL NOT_READY: AddSDKPort nao retornou porta valida` exit7; no P2pIsActive, no listener inspected, no NetSDK. Previous run had port returned ACTIVE but aborted at non-proven-loopback bind exit9. Do NOT assert specific cause. Hidden Cloud ID entry may differ, transient cloud/peer/status also plausible. New C# harness R2 asks user for exact same Cloud ID twice without echo (MATCH or mismatch exit11 BEFORE network), measures `AddSDKPort` elapsed_ms and emits closed category ZERO/NEGATIVE/OUT_OF_RANGE/VALID_PORT, categorizes invocation exceptions without vendor error text. Only one call, no retries or firewall relax. Old gate to stop non-loopback listeners remains. New R2 code Windows NOT BUILT/TESTED; next user command git pull + existing Connect script, provide sanitized ADD_PORT status/timing and bind categories. No vendor IDs or DLLs in repo. PR draft/main/F004 untouched, previous native R10 Windows 6/6 synthetic baseline unchanged.


## 46. Operator confirms repeatable vendor ACTIVE tunnel R2 with wildcard listener; R3 owner probe (28/10/26)

Latest observed user Windows output: hidden Cloud ID double-confirm MATCH, AddSDKPort elapsed_ms=1920 returned VALID_PORT, TUNNEL ACTIVE, route NOT_RELAY_REPORTED (NO direct claim), listener OBSERVED, BIND CATEGORIES WILDCARD_IPV4, BIND SCOPE NOT_PROVEN_LOOPBACK, safety stop exit9. This is second observed valid-port/ACTIVE, not a verified NetSDK connection; first output exit9, second exit7 transient, third exit9. No vendor secret/ID written repo. Port bound to all IPv4 interfaces (0.0.0.0) and potential LAN exposure, though firewall may block external access; cannot pass credentials yet. New lab R3 code uses Microsoft IP Helper API GetExtendedTcpTable, TCP_TABLE_OWNER_PID_LISTENER, to emit LISTENER OWNER category by matching dynamic port and process PID; no raw PID/IP. Safety gate preserved, no firewall changes, C++ VMS unmodified. Windows build R3 NOT RUN; next ask operator to run git pull and same lab. Then consider controlled temporary firewall isolation with explicit approval and remote reachability validation before any NetSDK auth. PR#4 draft, main/F004 Human Review preserved; R10 native 6/6 synthetic historic.


## 47. Owner Windows confirmed: Cloud library's listener belongs to LAB process; controlled isolation plan (28/10/2026)

User ran R3 on Windows after pulling 95a48f2; all five binaries matched prior signatures; hidden two-entry Cloud ID MATCH; AddSDKPort `VALID_PORT` at 1921ms; P2pIsActive ACTIVE; route NOT_RELAY_REPORTED (no Direct claim); GetActiveTcpListeners WILDCARD_IPV4; Win32 GetExtendedTcpTable owner CURRENT_LAB_PROCESS; safety stop exit9, so no NetSDK credentials transmitted. This confirms original SIM Next DLLs loaded in a *standalone harness process* and that process owns wildcard TCP listener, not that binaries can redistribute or that SIM Next service absent. 0.0.0.0 bind untrusted until LAN access proven blocked. Added design-only plan docs/foundation-005/F005-SIMNEXT-CLOUD-LAB-FIREWALL-ISOLATION-PLAN.md for temporary executable-scoped inbound TCP block rule before vendor DLL runs (requires explicit user approval and admin window), keep vendor DLL process non-elevated, check localhost works and external LAN host blocked before NetSDK test. No firewall commands executed, no code changes to native product, no Cloud credentials/serial entered in GitHub. R10 C++ 6/6 synthetic PASS, main/F004 Human Review untouched, PR draft.


## 49. User consent for temporary firewall: Prepare + IsolationProbe code shipped (28/10/2026)

User explicitly approved lab-only temporary inbound TCP BLOCK rule targeting Sentrium.CloudLibLab.exe with rollback. Branch has script scripts/windows/sentrium-cloud-lab-firewall.ps1 with Prepare (unprivileged .NET10 apphost Release build, UAC Install only), Status (validate rule exact filters and ActiveStore/Profiles), Remove (UAC, exact owned rule and refuses removal during running lab). C# tool has isolation-probe mode with own PID attribution and a no-credentials TCP localhost check, outputs dynamic port and holds while operator checks LAN from second authorized host. Wrapper `-Mode IsolationProbe` validates firewall Status and non-admin apphost, attempts to roll back temporary rule on process exit via UAC. Original -Mode Connect still stops on wildcard; no credential handling except Cloud ID input locally. User first runs git pull then `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action Prepare` from non-elevated VS Developer PowerShell, then later follow-up for IsolationProbe. No rules applied remotely by assistant, source changes NOT Windows tested, no P2P or NetSDK video completion claimed, PR4 draft and C++/main/F004 untouched.


## 50. Both Windows shells report admin token; patched firewall Prepare only (28/10/2026)

Operator attempted approved temporary inbound TCP BLOCK `-Action Prepare` in VS2022 Developer PowerShell, then ordinary PowerShell opened by Win+R. Both returned same early error `Prepare deve rodar SEM elevar` at line 74. Thus no build/firewall change happened from those calls, and no Cloud library loaded. User had already authorized temporary lab rule. Changed firewall script so `Prepare` allows elevated and standard shells, reports token category, compiles apphost .NET (no vendor DLL), invokes exact Install directly if elevated or UAC Install if standard, verifies ActiveStore. No change to scope/rollback. **IsolationProbe still requires non-admin Windows token and an external LAN block test**; if every shell is elevated, investigate UAC/standard user instead of bypassing. New source change not run on Windows. Next single command git pull + `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action Prepare` in existing PowerShell. No main/F004/product C++ changes, PR4 draft.


## 51. Firewall Prepare proved on Windows; DiagnoseToken read-only mode added (28/10/2026)

User's Windows PowerShell output after pulling 894c1823: F005 PREPARE TOKEN ELEVATED, net10.0 Release build success, firewall Block rule installed for lab program, `F005 FIREWALL STATUS PASS` x2 and `F005 PROTECT READY`. No vendor Cloud DLL loaded and no LAN-isolation proof yet. Rule persists on user's machine; remove only via scoped `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action Remove` after lab is stopped. All shell launch attempts so far showed WindowsPrincipal admin token, incl Win+R powershell. Do NOT bypass unprivileged-only IsolationProbe gate. New firewall script `-Action DiagnoseToken` read-only prints ELEVATED/STANDARD, UAC ENABLED/DISABLED/UNKNOWN, builtin administrator SID suffix -500 YES/NO, FilterAdministratorToken state without revealing identifying info. This determines safe path to a standard logon account/shell. New diagnostic not run on Windows yet; GitHub branch PR4 draft, no main/F004 C++ changes, no NetSDK/video proof.


## 52. Critical DiagnoseToken PowerShell parse corruption diagnosed and surgically fixed (29 Oct 2026)

Operator reported ParserError after `git pull` Already up-to-date 2b41fbd4 and -Action DiagnoseToken. GitHub file inspection showed a **repo-source defect**: the regex `-match '-500$'` was truncated at `'-500` and a full duplicated switch Prepare/Install/Status/Remove block inserted inside function Diagnose-Token, with dangling `)`. Caused parse failure before script actions; NOT related to closing the old console. Patch repaired exact broken span, restored correct regex, retained ONE switch and ONE case for each Action and prior protections. Static unique structural checks done in tool. Need operator Windows `[System.Management.Automation.Language.Parser]::ParseFile` gate before running read-only -Action DiagnoseToken; do not claim Win runtime PASS until received. Firewall rule prior Prepare succeeded and remains installed; no second Prepare required. No Cloud lib usage or device login, no C++ change, main/F004 Human Review preserved, PR4 draft.


## 53. Windows UAC gate conclusively diagnosed (29 Oct 2026)

User reran after fc409df6: native PowerShell Parser.ParseFile PASS and DiagnoseToken emitted CURRENT_TOKEN=ELEVATED, UAC=DISABLED, BUILTIN_ADMIN_ACCOUNT=NO, BUILTIN_ADMIN_APPROVAL_MODE=DISABLED. Original parser issue resolved; Win+R PowerShell elevated because UAC disabled for administrative user (not builtin Administrator). Never bypass standard-user-only Cloud library isolation probe, and never modify global UAC without operator action/approval. Recommended manual route Windows UserAccountControlSettings slider above Never Notify, reboot if enabling EnableLUA from 0, then open non-elevated PowerShell and recheck `-Action DiagnoseToken`, requiring UAC ENABLED+CURRENT_TOKEN STANDARD. If they cannot alter global OS policy, alternative standard local Windows user for lab. The previously user-authorized firewall BLOCK rule was created and validated; remains until Remove, no inbound port exposure opened. No second PC LAN network proof or NetSDK login yet. Only docs/handoff changes; native C++, main, F004 Human Review remain untouched, PR4 draft.


## 54. Windows standard token gate PASS after UAC re-enabled; proceed to existing IsolationProbe (29 Oct 2026)

User manually enabled UAC/rebooted, then ran `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action DiagnoseToken` from normal PowerShell, confirming `CURRENT_TOKEN=STANDARD`, `UAC=ENABLED`, `BUILTIN_ADMIN_ACCOUNT=NO`, builtin approval mode disabled (only builtin Administrator context, not relevant). The local repo's Git now reports dubious ownership because NTFS folder owned by BUILTIN/Administrators; no git updates from non-elevated token but the current local checkout has necessary IsolationProbe code from earlier. Do NOT suggest global safe.directory wildcard or unapproved takeown/ACL fix; skip git pull for next step. User next runs existing `scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode IsolationProbe` in PowerShell standard token. It checks firewall Status before starting vendor EXE; if blocked due to rule inactive after reboot do not execute vendor code. If accepted, local loopback handshake without NetSDK creds, keeps dynamic local port available for second authorized LAN PC Test-NetConnection (should False); Enter triggers rollback of temporary block rule via UAC. User should share redacted status lines only. Git PR4 draft, C++/main/F004 untouched, no NETSDK or actual video proof.


## 55. Windows F005 IsolationProbe localhost TCP PASS and scoped firewall rollback PASS (29 Oct 2026)

User ran IsolationProbe from standard Windows token after UAC enabled. Real log: F005 FIREWALL STATUS PASS; Cloud ID double-entry MATCH privately; AddSDKPort VALID_PORT in 1947ms; P2pIsActive ACTIVE; route NOT_RELAY_REPORTED (NOT proof Direct); socket WILDCARD_IPV4 owned CURRENT_LAB_PROCESS; ISOLATION LOOPBACK TCP CONNECTED; a dynamic port was shown only for test. User pressed Enter, and wrapper removed rule successfully, F005 FIREWALL REMOVED. Thus vendor SDK local listener/handshake observed with non-admin privilege and firewall installed, but no second-host LAN Test-NetConnection, no NetSDK login, no video or independent Cloud authentication validation. Rule is currently REMOVED; must reinstall with -Action Prepare before another test. Local Git dubious ownership unresolved; no pull needed to run existing scripts. Next when second authorized LAN computer available, prepare, launch isolation probe, test from peer while first terminal is open, only a FALSE result supports blocked LAN; rollback after. Main/F004 and C++ unchanged, PR draft.


## 56. Operator pivots from proprietary Cloud/T2U to live AutoRegistration video and AI/LPR (30 Oct 2026)

Owner explicitly postpones Cloud LAN isolation follow-up and wants real video via native Intelbras AutoRegistration ASAP, then LPR/video intelligence. F003 LAN NetSDK login/channel list validated; F005 AutoRegistration SERVER_CONN and UI present; F004 media lab `CLIENT_RealPlayEx` and PlaySDK decoded callback tests on LAN validated. New F005 commit 72c35404 adds NetSDK Adapter startLive/stopLive dynamic loading CLIENT_RealPlayEx/CLIENT_StopRealPlayEx, reuses existing session including auto registered, real handles and sanitized SDK failure. DeviceWorker stops live before disconnect/logout/AutoReg loss and at shutdown. Commit b4670326 adds GUI QWindow HWND preview one channel at a time, Qt signal/slot to worker, close cleanup, channel button. Commit 624813be improves UI to choose ▶ Extra or ▶ Principal and adds field acceptance doc F005-AUTOREG-LIVE-PREVIEW-R1.md. Live window separate from QML mosaic; handle started is NOT actual frame proof. **No Windows build/runtime yet**. User's checkout Git ownership mismatch from re-enabled UAC; propose per-command `git -c safe.directory=C:/Projetos/SENTRIUM VMS/sentrium-vms pull --ff-only`, no global trust or ACL changes. Next need compile in Windows VS Developer PowerShell, run and field-validate video with real registered NVR, then PlaySDK frame pipeline into Qt Quick mosaics for LPR/IA. Current Cloud firewall temporary rule successfully removed before pivot; no Cloud permissions or vendor DLL work. Keep PR4 draft, no merge main, F004 Human Review not approved.


## 57. Windows Release native preview compilation reaches link step without errors, runtime next (30 October 2026)

User supplied complete 2318-line CMake/MSVC transcript after using per-process Git safe.directory environment entries restricted to checkout/vcpkg. It showed F005 PREFLIGHT PASS, Qt6.12 MSVC and external Intelbras NetSDK/PlaySDK x64, CMake configure/generate done, C++ compile scanning and `[62/63] Linking CXX executable sentrium-client.exe`, returned to prompt with no error Cxxxx/LNK/ninja failure in transcript; optional Vulkan header lookup missing was non-fatal. Did not print `$LASTEXITCODE` or verify EXE physical path, so mark build as strongly indicated but not fully confirmed; runtime and actual video NOT RUN. Next validate `$LASTEXITCODE`, `out/sentrium-release/apps/sentrium-client/sentrium-client.exe`, set session PATH Qt/bin, NetSDK/bin and PlaySDK/DLL, launch client, configure existing already authorized NVR via AutoRegistration, test `▶ Extra` and `▶ Principal` channels with REAL camera, then observe cleanup. No vendor DLL redistribution, firewall/port forwarding, credential exposure or Cloud lab. main/F004 untouched, PR4 draft.


## 58. Runtime Qt real opens; AutoRegistration MAFRAN remains Connecting 0 channels (31 Oct 2026)

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


## 59. PR #5 UI r25 — nova Home, Explorer e Dispositivos (31/10/2026)

Desenvolvimento exclusivamente em `feature/r25-ui-parity` baseado no commit `afdc8db` da F005. Componentes QML `HomeDashboard.qml`, `DeviceExplorer.qml`, `DevicesWorkspace.qml` criados e integrados a `Main.qml`/CMake; nenhum backend C++/NetSDK alterado. Há pesquisa, expandir/recolher, favoritos temporários, menus de câmera/dispositivo, tabela e diagnóstico por canal, além de foco em tile com duplo clique sem diminuir a capacidade do mosaico. A r25 ainda não foi reproduzida por completo: grupos, persistência, drag/drop, abas de mosaico independentes, processamento IA/LPR e playback pendentes. **Teste de build Qt/QML e validação Windows ainda NÃO RUN**; PR #5 draft, PR #4 draft, main e F004 intactos. Mais detalhes e gate único em `docs/ui/R25-UI-IMPLEMENTATION-002.md`. O operador pediu entregas grandes, uma validação no final, sem repetições. A regressão AutoRegistro ainda precisa ser reproduzida com novo dispositivo, não foi corrigida por esta UI.


## 60. UI — árvore de dispositivos como r25, primeira alteração pedida pelo usuário (31/10/2026)

O usuário mandou capturas da lateral atual do SENTRIUM (cartão NVR Intelbras) e da r25 (sites/grupos, gravadores e canais indentados). Exigência: substituir cartões por árvore compacta **grupo → gravador → câmera**, começando por este módulo; liberar nova versão somente após aprovação. Feito em `feature/r25-ui-parity`, PR #5 draft: `DeviceExplorer.qml` com rows virtuais, grupos abertos/devices recolhidos por padrão, pesquisa, favoritos e menus; `DeviceController` agrega grupos e atribuições em memória, expõe dados sem tocar NetSDK, abre vídeo pelas ações já existentes; `Main.qml` vincula propriedades/sinais. Não usar nomes da r25 como dados artificiais. **Win Qt QML BUILD / RUNTIME / comparação visual NOT RUN.** Sem persistência; branch F005, F004, main intocadas. Leia `docs/ui/R25-DEVICE-TREE-R1.md` antes de continuar. Não criar release antecipado.


## 61. UI changes 2–4 requested by operator — header, search only, layouts (31/10/2026)

Operador solicitou 3 ajustes por captura: abas imediatamente após nome do produto para ganhar espaço, `×` sem preenchimento; lateral de dispositivos em Visualização deve exibir somente barra de pesquisa acima da árvore, retirando atalhos e status; menu de layouts r25 (grids 1,4,9,16,25,36,49,64 e destaques 6,8,10,12,14). Implementado na branch **`feature/r25-ui-parity`** draft, NOT TESTED Windows: `Main.qml` header integrado + popup de layouts + geometria tiles, `DeviceExplorer.qml` barra única e menus contextuais, novo `MosaicLayoutsPopup.qml`, `DeviceController.hpp/.cpp` com máximo 64 slots e validação das 13 capacidades, `CMakeLists.txt` atualizado. `WindowContainer` só criado em tiles ocupados via `Loader`; 64 streams/FPS/memória/estabilidade NÃO são PASS. **NÃO fazer release/merge antes da aprovação do usuário.** Detalhe: `docs/ui/R25-UI-CHANGES-002-004.md`. F005 base/main intactas.


## 62. UI modifications 5/6/7: contextual menus and camera intelligence indicators (31/10/2026)

Capturas da r25 e solicitações do operador: reproduzir menus contextuais da câmera, menus do gravador e badges no topo do tile sinalizando IA realmente ativa + canal selecionado para tabela Eventos ao vivo. PR #5 draft `feature/r25-ui-parity`: novo `CameraContextMenu.qml` compartilhado com ações reais fechar/trocar stream/favoritos e desabilitação explícita de áudio, captura, IA, etc.; árvore `DeviceExplorer.qml` com Canais → abrir Extra/Principal/fechar do gravador, Dispositivo → Info, categorias Stream padrão/Áudio desabilitadas; `DeviceController.closeDeviceChannels`; dados de IA `aiState=Unavailable` em F005, nunca tratar como ativa; seleção EVENTOS em memória sincronizada entre menu e badges da Visualização, sem eventos falsos, sem ativar engine do Server. Ainda sem build/hardware test. Documento `docs/ui/R25-UI-CHANGES-005-007.md`. Não gerar nova versão até usuário confirmar.


## 63. UI-008 — câmera/gravador por drag/drop (31/10/2026)

Solicitação: arrastar cada câmera para tile arbitrário, gravador para mosaico abrindo todos os canais em Extra. Branch `feature/r25-ui-parity` PR #5 draft: proxies QML `Drag` nas linhas de câmeras e gravadores conectados em `DeviceExplorer.qml`, `DropArea` para todos os tiles em `Main.qml`. Câmera substitui apenas tile escolhido; gravador preenche a partir daquele tile em Extra, expande grade até 64, preserva tiles não afetados, wrap se exceder posição 63; tile aberto pode mudar/trocar de posição ao arrastar cabeçalho, via novo `DeviceController::swapLiveSlots`. Payload somente IDs/index (não enviar credenciais). **Qt build / Windows RealPlay / Drag nativo / comportamento de WindowContainer ainda NÃO executados**, aceitar como código draft e não como entrega testada. Documento `docs/ui/R25-UI-CHANGES-008.md`. Nenhuma release, main/F005/F004 intactas.


## 64. RC da interface r25 aprovado para compilação, não para merge (31/10/2026)

Usuário solicitou encerrar mudanças 1–8 e **aplicá-las**. Branch RC congelada: `release-candidate/visualizacao-r25-rc01` derivada do commit `30c35f30f298426719d338e92aafe3e28c661a1e`. Arquivo de release notes `docs/ui/SENTRIUM-R25-VISUALIZACAO-RC01.md`. Integração de árvore, abas topo, busca, layouts, menus contextuais, badges IA/EVENTOS, drag/drop sob NetSDK foi escrita no GitHub. **Versão candidata de código, não exe gerado/homologado**. Próxima etapa: operador baixar branch, compilar com helper MSVC Qt aprovado, enviar erros se houver e testar funcionalidade real. GitHub PR #5 segue draft, main e foundation/005 continuam intactas para preservar versão anterior. Não repetir desenvolvimento do UI-001..008 sem relato de falha, nem afirmar que compilou antes de logs. Todos os testes Cloud/PTCP anteriores continuam inalterados.


## 65. Pós-RC01 correção de menu/câmera tile e reposicionamento (01/11/2026)

Relato real do operador na RC01: não abre menu do tile e drag tile→tile não funciona. Qt 6.12 Documentação do WindowContainer confirma que a janela nativa pode cobrir QML: abordagem priorizada em `feature/r25-ui-parity` apenas. `CameraContextMenu.qml` popupType Popup.Window; Main.qml fornece botão `⋯` no cabeçalho, TapHandler right click e popup(x,y) explícito, além de onReleased do MouseArea e tileAt(x,y) com Repeater.itemAt() para mover/trocar tile sem depender de DropArea sob HWND. DropArea mantém chaves só câmera/gravador; CMake feature trouxe cachegen skip hotfix. Revisão estática de blocos passou. **SEM build/teste Windows pós-correção, não alegar funcional/APPROVED.** Não avançar RC/main até o usuário autorizar nova versão. Documento `docs/ui/R25-UI-POST-RC01-IMPROVEMENTS.md`.


## 66. AutoRegistro falha após primeira conexão — correção em desenvolvimento (01/11/2026)

Relato: NVR conectado uma vez não reconecta ao SENTRIUM sem salvar config no gravador, diferente do SIM Next. **Causa verificável no código:** callbacks de desconexão/reconexão NetSDK estavam com corpos vazios; Worker ignorava registro se `sessions_` tinha handle não marcado stale, e falha de logout podia bloquear todas tentativas posteriores. Em `feature/r25-ui-parity`, codificados: `Adapter::setSessionStatusCallback` + callbacks nativos encaminhados com mutex a sinal Qt queued, `DeviceWorker` com QTimer worker a cada 15s e último endpoint cadastrado em memória, fallback de LoginEx2 SERVER_CONN, aceitação de novas registrations e aposentadoria da sessão obsoleta, tratamento de callback duplicado, cancelamento por logout manual; `DeviceController` guarda tiles/stream e restaura após login confirmado, previne ressuscitar explicitamente desconectado, e `startLive` corrigido 16→64. **NetSDK documentação avisa que auto registration login pode NÃO suportar auto-reconnect**, portanto não confiar nele isoladamente. Próximo passo: autorização de nova RC de teste e build Qt/MSVC (ainda NOT RUN), testar queda/retorno 2x sem salvar config no NVR, verificar Connected→Connecting→Connected e recuperar mosaico. UI posterior RC01 (cabeçalho, placeholder, menu de tile e drag) também na feature. Documento `docs/foundation-005/F005-AUTOREG-RECOVERY-RC02.md`. Nenhuma alteração em RC01, F005 base, main ou Cloud/P2P; não fazer merge/release sem autorização do operador.


## 67. SENTRIUM Visualização r25 RC02 para teste — 03/11/2026

Usuário disse “VAMOS SEGUIR” após receber proposta de RC02. Foi criada branch `release-candidate/visualizacao-r25-rc02`, sem tocar main ou RC01, com identificaçao no título da janela. Inclui 8 ajustes UI da r25, 3 melhorias posteriores, recuperação de Registro Automático (SDK callbacks, retry Worker 15 s, expiração de sessão, restauração do mosaico) e suporte 64 slots Worker. Verificações estáticas de presença das peças PASS, **sem build no Windows, sem teste com NVR ou aceitação funcional**, e sem garantia de reconexão se não houver novo evento e o endpoint não estiver válido. O NetSDK Programming Guide explicita que login por auto registro não suporta auto-reconnect nativo; por isso há retry no aplicativo. **Persistência de dispositivos não implementada**. O arquivo `docs/ui/SENTRIUM-VISUALIZACAO-RC02-VALIDATION.md` contém instruções e critérios. Não mesclar para F005/main até compilar e validar, deixar PR #5 draft. Teste primeiro com NVR não crítico, sem alterar sua configuração de registro.
