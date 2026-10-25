# Roadmap proposto — nenhum passo implementado

| Etapa | Escopo | Aceite antes da próxima etapa |
|---|---|---|
| 0 — decisão | revisar auditoria, donos e licenças | ADRs explicitamente aprovadas; artefatos SDK/modelos encaminhados |
| CPP-001 | somente scaffold descrito na arquitetura; C++20/toolchain, core/platform, health mínimo | build Debug/Release, CI, testes básicos e raiz v2-dev obrigatória; sem tocar v1; interfaces xc_cloud/xc_intelbras sem exigir DLLs ou SIM Next |
| 2 — contratos/segurança/storage | fixtures sintéticas, JSON/SQLite, segredos, RBAC mínimo e logs | corpus semântico, schema/UTC/erros, rejeição explícita de raiz v1 |
| 3 — SDK | login local, canais, live, PTZ e teardown | kit C++ já localizado: validar compatibilidade/EULA; xc_cloud separado; callback/lifetime e IB-01..06 antes de Cloud em produção |
| 4 — laboratório mídia/UI | PlaySDK versus FFmpeg software/D3D11VA/NVDEC/QSV; Quick/Widgets/WinUI | mesmas entradas, contadores, latência e qualidade; nenhum swap de produção automático |
| 5 — live/render | demux/decode, filas e mosaicos | paridade 1/4/16/64, layouts foco, DPI, stream fixo/automático, estabilidade |
| 6 — IA | ORT C++, pré/pós, tracker, ROI/linhas e ações | goldens por estágio, tolerâncias documentadas, providers comparados |
| 7 — LPR | detector/OCR nativos e consensus/gate | pesos/YAML/licenças; corpus dia/noite/reflexo, BR/variante/qualidade/latência |
| 8 — eventos/evidência | regras, retenção, mídia e contratos remotos | 151 requisitos + novos testes; HTTP ranges, pins e concorrência |
| 9 — integrações | LAP/outbox/gateway e futura homologação Official | Unknown seguro, auth/TTL, falhas parciais, homologação antes de habilitar |
| 10 — ampliação | listas, usuários completos, notificações, gravação/NVR pendentes | requisitos e migrações versionadas separadas da paridade |
| 11 — release | instalação, upgrade, recuperação, observabilidade e soak | SBOM, Debug/Release, hardware real, 24–72h, backup/rollback ensaiados |

## Laboratório que prova ou rejeita a migração

A: r25/PlaySDK atual; B: C++/PlaySDK; C: C++/FFmpeg software; D: C++/FFmpeg D3D11VA, com NVDEC/QSV quando elegíveis. Mesmos arquivos/streams, resolução, codec, FPS, layout, regras e modelos; warm-up e repetições. Separar custo de linguagem, decode e render. Medir CPU do host/processo, working/private bytes, handles, threads, GPU decode/3D/copy/compute, VRAM, cópias CPU/GPU, frames recebidos/decodificados/exibidos/descartados, queue depth, idade do frame e p50/p95/p99 ponta a ponta.

Cobrir live sozinho, live+IA, headless IA/LPR, múltiplos perfis, maximização, reconexão, CPU-only, GPU sem suporte, perda de rede, backpressure e shutdown. LPR medir leituras por placa/evento, precisão/recall e latência, não confundir FPS de entrada com placas reconhecidas. Metas numéricas são aprovadas ANTES da execução; guardar CSV bruto, versões, hardware, drivers, hashes e cenários. Nenhuma melhora foi medida nesta auditoria.

Rollback: v1/r24/r25 imutáveis; dados e instalação v2 isolados. Importação futura somente por cópia autorizada offline, backup verificável e comparação semântica; nunca apontar v2 ao DataRoot v1. Falha de gate mantém pipeline anterior e registra decisão.

Revisão documental Intelbras de 23/10/2026: [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md). Preservados demais resultados.