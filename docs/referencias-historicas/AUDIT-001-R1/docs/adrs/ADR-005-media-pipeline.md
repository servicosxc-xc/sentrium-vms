# ADR-005 — media-pipeline

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: FFmpeg candidato, PlaySDK oracle.

Alternativas: PlaySDK nativo pode permanecer se formato/paridade invalidar FFmpeg.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: demux RAW/DHAV, timestamps, snapshot/playback e reconexão reais.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).