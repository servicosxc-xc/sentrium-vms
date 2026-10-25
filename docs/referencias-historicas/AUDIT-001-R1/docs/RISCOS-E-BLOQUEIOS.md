# Riscos e bloqueios

Todos pendentes de validação, sem correção aplicada à r25. P0 bloqueia a capacidade indicada, não necessariamente um scaffold isolado futuro. Responsáveis propostos: arquitetura/SDK, segurança, mídia, IA/LPR e jurídico, a designar pelo projeto.

| Prioridade | Risco | Evidência/critério de encerramento |
|---|---|---|
| Resolvido tecnicamente | NetSDK/PlaySDK C++ x64 disponíveis | pacote General_NetSDK_3.050_PlaySDK_3.042; disponibilidade não é blocker, compatibilidade/licença seguem gates |
| Resolvido tecnicamente | caminho libt2u nativo e Direct/Relay identificados | wrapper .NET não é obrigatório; ver addendum para evidência e limites |
| P0 produção | IB-01/02/03: redistribuição libt2u, bootstrap oficial e uso standalone | autorização, configuração/credential oficial e confirmação Intelbras |
| P1 produção | IB-04/05/06: Windows limpo, reconexão/failover e SDK antigo | ensaios reais sem SIM Next; matriz de compatibilidade e paridade |
| P0 | Formato RAW/DHAV não é Annex B garantido | capturas sintéticas/autorizadas e demux validado antes de substituir PlaySDK |
| P0 | Pesos/configuração OCR e licenças não fechados | hash, YAML, alfabeto, dimensões, modelo/card e aprovação; bloqueia paridade LPR |
| P0 | ASP.NET DataProtection não é raw DPAPI | estratégia de leitura/exportação autorizada e fixture isolada; nunca tentar decriptar produção nesta etapa |
| P0 | Official Muralha bloqueado | homologação/contrato de envio; Unknown sem retry cego |
| P1 | SignalR não equivale a WebSocket Drogon | implementar protocolo compatível ou versionar cliente/servidor juntos com plano explícito |
| P1 | Zero-copy/performance não demonstrados | medir CPU/GPU copies, p95/p99, 64 streams, AI/LPR e soak |
| P1 | Qt/licenças/patentes/providers | SBOM e parecer de distribuição antes de compromisso de release |
| P1 | SDK callbacks e shutdown | ownership, geração, unsubscribe, espera de callbacks e teste de desconexão/teardown |
| P1 | Fronteira multiprocesso | decoder/textura não se compartilham por suposição; IPC ou duplicação contabilizada |
| P1 | Credenciais e autorização HTTP | não reproduzir exposição de health/paths; autenticação, escopos e auditoria desde início |
| P1 | Lacunas de UI mascaradas por menu | snapshot contextual/áudio/talk/gravação/instant playback não viram requisitos dados como prontos |
| P1 | Automático legado e docs divergentes | preservar promoção ao maximizar, sem mudar modos fixos; golden obrigatório |
| P1 | 151 testes não provam campo | relatórios anteriores não são execução nesta auditoria; acrescentar nativos, stress e golden |
| P2 | Inventário lexical não é AST | revisão humana e compilação futura; símbolos dinâmicos/reflection exigem investigação |
| P2 | Operação externa pode alterar log r25 | comparar manifestos integrais e reportar divergência, sem parar serviço ou mascarar hash |

Decisão: GO WITH BLOCKERS somente para revisão e eventual autorização posterior de CPP-001. Não é GO de produto ou de performance. A auditoria termina sem iniciar implementação. Ver os relatórios temáticos para arquivos e contratos rastreáveis.

## Revisão Intelbras

Os seis gates IB-01..06 do [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md) não impedem Foundation C++ desacoplada futura. Integração Cloud não está pronta para produção. Nenhum outro risco foi encerrado nesta revisão.