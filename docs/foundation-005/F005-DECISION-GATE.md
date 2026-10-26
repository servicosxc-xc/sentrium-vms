# F005 — DEVICE CONNECTIVITY — DECISION GATE 0.1

**Estado:** **F005-AUTH-001 — AUTHORIZED FOR EXECUTION**  
**Ato humano:** após concordar que a F004 estava presa a testes de NVR incompleto, o responsável confirmou expressamente “vamos seguir” para priorizar F005 interface, Cloud/P2P e Registro Automático em 25/10/2026.

## Ordem obrigatória do executor

**Gate 0A — PRE-FLIGHT READ-ONLY (específico de conectividade):** confirmar Git, branch, toolchain, Qt e NetSDK/PlaySDK já existentes; localizar/inspecionar kit e wrappers Cloud/P2P legados e headers oficiais do NetSDK para Registro Automático. Não baixar SDKs. Não coletar credenciais nem criar listeners no preflight. Separar conclusões `VERIFIED` / `MISSING` / `INCONCLUSIVE` por modalidade.

**Gate 0B — IMPLEMENTAÇÃO INCREMENTAL:** após pré-verificação do ambiente, avançar imediatamente em mudanças de código da interface e preservar LAN; backend Cloud/Auto somente com API real verificada. Se falta SDK Cloud ou API Auto, registrar `BLOCKED` apenas na modalidade correspondente; nunca bloquear melhorias seguras de UI/LAN.

**Gate 0C — INTEGRAÇÃO FUNCIONAL:** executar somente smoke/regressão essencial por entrega; o teste de escala 16/32 câmeras depende de gravadores completos alcançáveis. Não transformar ausência de hardware em reprovação de arquitetura.

**Gate 0D — EVIDÊNCIA E HANDOFF:** commit incremental, BuildId, prova de conexão real por modalidade disponível, árvore de canais, diagnósticos sanitizados, pendências e instruções de uso. A prioridade é funcionalidade entregue, não relatórios ou benchmark repetitivo.

## Governança

- F004 é dependência técnica e permanece **IN IMPLEMENTATION / HUMAN REVIEW PENDING**. Sua ausência de encerramento não é um atalho para aprová-la.
- Esta branch começa no último checkpoint F004; **não fazer merge em `main`**.
- ADR-005 permanece **Proposed**; a F005 não o aceita automaticamente.
- Cloud/P2P e Registro Automático **não são declarados PASS** sem equipamento real.
- Não alterar firewall, encaminhamento de portas, políticas de rede, dados de produção ou SDKs externos sem autorização específica.
