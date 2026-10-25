# SENTRIUM-CPP-Foundation-002 — HUMAN REVIEW

**Registro:** F002-HUMAN-REVIEW-001  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-002 — Application Bootstrap  
**Data da decisão:** 25/10/2026  
**Estado:** **APPROVED**

## 1. Decisão humana

Após revisão da execução real da Foundation-002, incluindo interface gráfica, builds, testes, lifecycle e observação de idle, o responsável pelo projeto declarou:

**FOUNDATION-002 — HUMAN REVIEW APPROVED**

A implementação da Foundation-002 está formalmente aceita.

## 2. Alcance da aprovação

A aprovação cobre o objetivo técnico da Foundation-002:

- primeiro cliente gráfico C++ real;
- Qt 6.12.0 em uso experimental;
- shell funcional da aplicação;
- lifecycle;
- logging;
- DataRoot isolado;
- builds Debug e Release;
- testes automatizados;
- smoke gráfico;
- observação de idle.

## 3. Design visual

O shell visual atual foi aprovado **como bootstrap funcional da Foundation-002**, não como design definitivo do SENTRIUM VMS.

O responsável registrou que o design atual está diferente da direção visual desejada e que deverá evoluir nas Foundations posteriores.

Portanto:

- a aprovação da F002 não congela o layout atual;
- a aprovação da F002 não transforma placeholders em UX definitiva;
- a direção visual profissional previamente definida continua válida;
- refinamento visual deverá ocorrer sobre funcionalidades reais, especialmente dispositivos, vídeo e mosaico.

## 4. ADR-004

**ADR-004 — client-framework permanece Proposed.**

A aprovação da Foundation-002 não promove automaticamente o Qt a decisão arquitetural definitiva para toda a aplicação ou pipeline de mídia.

Qt Quick permanece experimental e reversível até que as etapas de mídia produzam evidência suficiente.

## 5. Limites

Esta aprovação não autoriza implicitamente:

- NetSDK;
- PlaySDK;
- libt2u;
- Cloud/P2P;
- FFmpeg;
- decode;
- gravação;
- playback real;
- IA;
- LPR;
- banco operacional;
- IPC;
- HTTP/WebSocket;
- qualquer Foundation posterior sem escopo próprio.

## 6. Resultado

**SENTRIUM-CPP-Foundation-002 — CLOSED / PASS / HUMAN REVIEW APPROVED**
