# Decisão executiva — GO WITH BLOCKERS

Recomenda-se revisar e autorizar separadamente um futuro CPP-001 mínimo. Não iniciar implementação agora e não prometer ganho de desempenho. A r25 é oracle funcional; wrappers gerados não devem determinar arquitetura por tradução literal.

C++20, VS2022/v143 x64, CMake/Ninja/vcpkg com versões fixadas; dez fronteiras de bibliotecas por responsabilidade, incluindo xc_cloud separado de xc_intelbras e duas aplicações, conforme [arquitetura](ARQUITETURA-CPP-PROPOSTA.md). Drogon é candidato adequado ao controle HTTP assíncrono; trabalho pesado fica em executores limitados. SignalR exige solução explícita. Qt Quick é preferência condicionada a benchmark e licença, comparada a Widgets/render nativo e WinUI.

FFmpeg é candidato a demux/decode, primeiro validado contra RAW/DHAV real. D3D11VA tem prioridade de investigação Windows; software é fallback, NVDEC/QSV opcionais. Não presumir zero-copy ou decoder compartilhado entre processos. PlaySDK segue referência comparativa até evidência.

ORT C++ permite retirar C#/Python dos caminhos IA/LPR, preservando pré/pós-processamento, tracker, gate, consenso e thresholds. OCR instalado é slot/alphabet, não CTC genérico. Pesos/configuração/licenças ainda bloqueiam fechamento LPR. SQLite pode preservar schema/contratos; DataProtection precisa tratamento compatível específico e raiz v2-dev isolada.

Bloqueios principais: licença libt2u, bootstrap oficial, standalone/Windows limpo, reconexão/failover e compatibilidade do kit Intelbras antigo, formato mídia, modelos/YAML e licenças, compatibilidade SignalR/DataProtection, homologação Official, benchmark/UI e hardware real. [Riscos](RISCOS-E-BLOQUEIOS.md), [roadmap](ROADMAP-CPP.md), [licenças](DEPENDENCIAS-E-LICENCAS.md). As dez ADRs estão Proposed, não Accepted.

Revisão documental Intelbras de 23/10/2026: [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md). Preservados demais resultados.