# ADR-003 — server-framework

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: Drogon para control plane.

Alternativas: Boost.Beast/Asio ou outro framework requerem mais plumbing.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: HTTP/JSON/ranges/TLS e SignalR resolvidos; tarefas pesadas fora do event loop.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).