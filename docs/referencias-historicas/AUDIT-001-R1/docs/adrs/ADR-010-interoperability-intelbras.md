# ADR-010 — interoperability-intelbras

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: NetSDK/PlaySDK C++ disponíveis com adaptador xc_intelbras; xc_cloud independente sobre libt2u nativa.

Alternativas: reconstruir headers por P/Invoke não é alternativa aceitável.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: disponibilidade headers/libs já resolvida; validar subset/ABI/compatibilidade e IB-01..06 para Cloud em produção. Foundation desacoplada pode anteceder esses gates, sem iniciar implementação nesta revisão.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).