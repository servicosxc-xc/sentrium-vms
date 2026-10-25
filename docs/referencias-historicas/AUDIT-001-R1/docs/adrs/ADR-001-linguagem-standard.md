# ADR-001 — linguagem-standard

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: C++20 e RAII.

Alternativas: C++17 reduz recursos de concorrência; C++23 aumenta exigência do toolchain.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: compilação MSVC x64, sanitizadores disponíveis, cancelamento/lifetime demonstrados.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).