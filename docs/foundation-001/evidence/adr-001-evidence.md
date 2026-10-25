# ADR-001 — EVIDENCE COLLECTED / REVIEW REQUIRED

Estado governado pela ordem R1: **Proposed**. Nenhum arquivo ADR foi alterado ou declarado Accepted.

Evidências iniciais: MSVC v143/x64 exato; compilações C++20 Debug/Release do zero; U-4/I-6 ownership/RAII/lifetime; U-5/I-5 cancelamento; I-1/I-6/I-10 shutdown/processo. Os dois ciclos completos passaram com 19/19 testes por configuração. Ver test-results.md, test-summary.json, architecture-check.md e arquivos brutos das execuções.

ASan: avaliado e indisponível na instalação aprovada (ausência de libs/runtime). Não foi instalado componente adicional nem executado teste instrumentado. sanitizer-evaluation.json registra a inspeção. A comprovação por sanitizador permanece pendente para revisão humana; os testes funcionais não a substituem. AC-23 cobre a entrega de evidências **iniciais**, não a aceitação do ADR.
