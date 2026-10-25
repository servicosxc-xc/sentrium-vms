# ADR-007 — ai-runtime

Status: **Proposed**

Contexto: migração da r25, com paridade e rollback imutável; auditoria sem implementação.

Decisão proposta: ONNX Runtime C++.

Alternativas: providers CPU/CUDA/TensorRT/DirectML/WinML comparados sem promessa.

Consequências: exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

Gate para Accepted: goldens YOLOX/tracker/ROI, versões e drivers compatíveis.

Referências: [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).