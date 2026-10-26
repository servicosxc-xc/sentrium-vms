# F004-MEDIA-LIFECYCLE-001 — Três ciclos reais NetSDK/PlaySDK

**Projeto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** 004 — Media Lab  
**Data:** 25/10/2026  
**Branch:** `foundation/004-media-lab`  
**Resultado:** **PASS FUNCIONAL — 3/3 ciclos, sem processos residuais**  
**Fonte:** transcrição PowerShell enviada pelo operador; sem captura direta dos executáveis ou logs completos.

## Ambiente / procedimento

- Workspace: `C:\Projetos\SENTRIUM VMS\sentrium-vms`.
- Após `git pull --ff-only origin foundation/004-media-lab`, HEAD local atualizado para `11827d531c41f564aaf6bb7600f6d31f5f317de0`; alteração documental apenas.
- Binário: `out\sentrium-release\sentrium-media-lab.exe`, anteriormente compilado com Release PASS.
- NetSDK LAN: equipamento real já validado, porta `7000`; host IP não reproduzido neste relatório.
- Perfil: Principal, canal índice `0`, duração configurada `10 s`.
- `SENTRIUM_MEDIA_CHANNELS` e `SENTRIUM_MEDIA_CAPTURE` removidos do ambiente para este cenário.
- Usuário/senha não constam neste documento. Senha solicitada localmente com `Read-Host -AsSecureString`, passada ao processo temporariamente por ambiente e removida ao final.
- Cada execução do laboratório produziu JSON que foi convertido em PowerShell; depois foi consultado `Get-Process -Name sentrium-media-lab`.

## Resultados observados

| Medida | Ciclo 1 | Ciclo 2 | Ciclo 3 |
|---|---:|---:|---:|
| `Result` | PASS | PASS | PASS |
| ExitCode | 0 | 0 | 0 |
| `DecodedCallbacks` | 149 | 149 | 150 |
| `QueueDropped` | 0 | 0 | 0 |
| `InputFailures` | 0 | 0 | 0 |
| Processos residuais consultados | 0 | 0 | 0 |

**Total decodificado:** 448 callbacks. **Taxa aproximada:** ~15 callbacks/s por execução de 10 s, não taxa de renderização.

## Interpretação

- Conexão LAN NetSDK e decodificação PlaySDK passaram funcionalmente em três execuções sucessivas, com ExitCode 0.
- Nenhum erro de fila ou alimentação de decoder nas métricas expostas; processos do Media Lab inexistentes após cada retorno ao PowerShell.
- Evidência fortalece `F004-AC-07` (teardown determinístico) e `F004-AC-17` (ausência de processo residual **nesses três ciclos**).
- Não é teste de estresse prolongado, monitoramento de handles/threads entre ciclos, nem prova de ausência de memory leak.
- BuildId interno de cada execução não foi reproduzido pelo resumo apresentado; registrar como lacuna de rastreabilidade fina até apresentar JSON bruto sanitizado.
- A validação posterior de consumo do processo PlaySDK e recomendação da pipeline ainda são pendentes.

## Governança

`FOUNDATION-004 — AUTHORIZED / IN IMPLEMENTATION`.  
ADR-005 — `Proposed`.  
F005 — `NOT AUTHORIZED`.  
Sem merge para `main` e sem `HUMAN REVIEW APPROVED`.
