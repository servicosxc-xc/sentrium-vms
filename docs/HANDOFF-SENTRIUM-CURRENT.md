# SENTRIUM VMS — HANDOFF PERMANENTE / ESTADO ATUAL

**Atualizado em:** 26/10/2026  
**Projeto:** SENTRIUM VMS — Intelligent Video Management System  
**Repositório:** `servicosxc-xc/sentrium-vms` (GitHub)  
**Branch ativa:** `foundation/005-device-connectivity`  
**PR de desenvolvimento:** https://github.com/servicosxc-xc/sentrium-vms/pull/4 (draft, sem merge)
**Regra:** após abrir um novo chat, conferir `git rev-parse HEAD` no GitHub antes de prosseguir. O HEAD atual da branch é a fonte de verdade; este texto informa as decisões, não substitui o Git.

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

## 6. ÚLTIMO LOG DE COMPILAÇÃO DO USUÁRIO (26/10/2026)

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

> Continue o desenvolvimento **SENTRIUM VMS** a partir do repositório GitHub `servicosxc-xc/sentrium-vms`, branch `foundation/005-device-connectivity`. **Leia integralmente `CONTINUE-HERE.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` no GitHub e confira o HEAD real**. Não recomece fundamentos nem repita benchmarks. O Registro Automático já conectou um NVR real com 8 canais; Cloud P2P nativo está em desenvolvimento, sem SIM Next e sem executar `dh-fwd` (bloqueio Defender). O último build de 26/10 falhou por **LNK1168 (cliente aberto)**, não por diagnóstico confirmado de C++. Já existe um script único de build/validação `scripts/windows/sentrium-validate-f005-cloud.ps1`. **Trabalhe em blocos grandes**, aplique código diretamente na branch, preserve `main`, atualize handoff e forneça comandos só ao fim.

## 11. Último resultado e correção do PRE-FLIGHT (26/10/2026)

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
