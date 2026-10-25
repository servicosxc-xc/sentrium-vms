# ADR-009 — persistence

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: SQLite com schema compatível e RAII.

Alternativas: ORM pesado ou troca de banco não justificados.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: backup WAL, concorrência, JSON/UTC, v2-dev e compatibilidade de segredos.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).