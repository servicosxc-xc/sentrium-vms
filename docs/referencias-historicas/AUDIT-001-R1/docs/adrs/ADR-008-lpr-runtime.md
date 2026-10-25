# ADR-008 — lpr-runtime

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: Pipeline nativo fiel ao detector/OCR observado.

Alternativas: bridge Python pode servir de oracle isolado, não objetivo final.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: pesos/YAML/licenças e corpus detector/OCR/normalização/consenso aprovados.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).