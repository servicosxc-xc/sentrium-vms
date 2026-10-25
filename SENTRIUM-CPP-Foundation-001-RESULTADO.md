# SENTRIUM-CPP-Foundation-001 — RESULTADO

**FOUNDATION-001 — PASS / READY FOR HUMAN REVIEW**

## A. Identificação

- Produto: SENTRIUM VMS — Intelligent Video Management System.
- Foundation: SENTRIUM-CPP-Foundation-001.
- Ordem executada: FINAL R1; SCOPE 1.1 informado como APPROVED; CONTEXT-MANIFEST 1.0 informado como baseline validada; F001 Decision Gate AUTHORIZED.
- Data: 24/10/2026, America/Sao_Paulo.
- Versão: `0.0.0-foundation001`; BuildId: `source-b8b6d42374064dc8`.
- Workspace sem repositório/commit Git. Identidade derivada dos hashes das fontes, não de um commit fictício.
- Raiz autorizada: `C:\Projetos\SENTRIUM VMS\SENTRIUM-CPP-Foundation-001`.

## B. PRE-FLIGHT

**PREFLIGHT PASS**, executado inicialmente só com inspeções de leitura, antes de criar arquivos. Workspace vazio, raiz e ancestrais sem reparse points. Windows NT 10.0.26200.0. Instalação oficial localizada por vswhere: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`, instância 563a9b85, completa e inicializável.

| Componente | Esperado R1 | Detectado | Resultado |
|---|---|---|---|
| Build Tools | 17.14.41 / 17.14.37710.0 | Exato | PASS |
| Platform Toolset / família | v143 / 14.44 | Exato | PASS |
| VCToolsVersion | 14.44.35207 | Exato | PASS |
| cl.exe FileVersion | 19.44.35229.0 | Exato | PASS |
| link.exe FileVersion | 14.44.35229.0 | Exato | PASS |
| Host / target | x64 / x64 | Exato | PASS |
| SDK produto | 10.0.26100.9457 | Exato; pacotes MSI 10.1.26100.9457 | PASS |
| CMake | 4.4.4 | Exato | PASS |
| Ninja | 1.13.2 | Exato | PASS |
| vcpkg registry/tag | 2026.10.16 | Tag aponta para o HEAD aprovado | PASS |
| vcpkg commit | Valor aprovado declarado em vcpkg.json | Igualdade exata HEAD/manifest/tag | PASS |
| vcpkg-tool | 2026-09-26 | 2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b | PASS |

Compilador/linker efetivos estão sob `VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64` dessa instalação. VsDevCmd inicializou x64/x64 somente no processo. O SDK usa diretórios Include/Lib/bin `10.0.26100.0`; a revisão de servicing foi comprovada por pacotes e rc.exe, sem confundir os identificadores.

Ferramentas portáveis verificadas em `../_toolchain`; nenhuma instalada ou atualizada pela Foundation. [PRE-FLIGHT detalhado](docs/foundation-001/evidence/preflight-report.md), [inventário bruto](docs/foundation-001/evidence/sentrium-final/preflight.json), [ambiente](docs/foundation-001/evidence/sentrium-final/msvc-environment.txt), [hashes](docs/foundation-001/evidence/sentrium-final/toolchain-hashes.json).

## C. Implementação realizada

Criados diretamente na raiz CMakeLists.txt, CMakePresets.json e vcpkg.json; fontes em apps/libs/tests; automação em scripts/windows; evidências em docs/foundation-001/evidence. Sem pasta SENTRIUM intermediária.

| Target | Responsabilidade |
|---|---|
| sentrium-core | Configuração tipada, erros, estados/health, relógio, telemetria, JSON/logging, atividade cooperativa |
| sentrium-platform-win | Known Folder, handles RAII, DataRoot validado, arquivo/lock/logs limitados e bootstrap/shutdown |
| sentrium-server | Entrada CLI local |
| sentrium-unit-tests | Oito cenários do core |
| sentrium-integration-tests | Onze cenários de filesystem/processo/binário |

Nenhuma biblioteca externa de produto. Arquivos principais: [core](libs/sentrium-core/core.cpp), [plataforma](libs/sentrium-platform-win/platform.cpp), [CLI](apps/sentrium-server/main.cpp), [build local](scripts/windows/sentrium-build.ps1). Nenhuma funcionalidade de mídia, UI, API, IA, LPR ou Foundations seguintes foi criada.

## D. Arquitetura

Dependências: `sentrium-server → sentrium-platform-win → sentrium-core`. Core sem APIs Windows ou dependência de plataforma. Handle movível para recursos Windows; unique_ptr para ownership exclusivo da atividade; std::jthread/stop_token com latch/condition_variable_any e join determinístico. Estados Starting, Ready, Stopping, Stopped e Failed, com erros tipados e exit codes não zero.

Shutdown fecha a admissão, registra a solicitação, solicita cancelamento, aguarda join, libera a atividade, finaliza logging e retorna. A política de paths valida destinos por handle antes de escrever. [Arquitetura e verificação PE](docs/foundation-001/evidence/architecture-check.md).

## E. Builds

Debug e Release configurados em diretórios independentes vazios; C++20/MSVC v143/x64; CRT /MDd e /MD respectivamente; /W4 /WX em código próprio. Configure/build/test/imports/headers retornaram 0 nas duas execuções finais. Zero warnings detectados nos quatro logs de build aprovados.

| Ciclo | Debug | Release |
|---|---|---|
| sentrium-final | Build do zero aprovado | Build do zero aprovado |
| sentrium-reproduction | Novo build do zero aprovado | Novo build do zero aprovado |

Os builds do primeiro ciclo aprovado foram arquivados dentro de out antes da reprodução. Os ativos estão em out/sentrium-debug e out/sentrium-release. Logs completos, comandos JSON, presets, exit codes e hashes em [sentrium-final](docs/foundation-001/evidence/sentrium-final) e [sentrium-reproduction](docs/foundation-001/evidence/sentrium-reproduction). [Warnings](docs/foundation-001/evidence/warnings-check.json).

## F. Testes

**19/19 por configuração e por ciclo: 8 unitários + 11 de integração.** Quatro suítes finais, 76 execuções de casos aprovadas. Sem teste desativado. A [lista individual](docs/foundation-001/evidence/test-results.md) mapeia U-01–U-08 e I-01–I-11 aos nomes CTest. JUnit, LastTest.log e saída verbosa foram preservados; [resumo estruturado](docs/foundation-001/evidence/test-summary.json).

Falhas anteriores preservadas: erro de tratamento de array JSON no script antes do configure; classificação inicial incompleta da DLL MSVC atomic-wait em I-9. Ambas corrigidas dentro da validação, sem alterar o lock ou permitir SDK proibido. O primeiro I-9 reprovado permanece no JUnit inicial. [Histórico](docs/foundation-001/evidence/sentrium-validation-classification.md).

Sanitizador: bibliotecas/runtime ASan ausentes na instalação aprovada. Não houve instalação nem execução instrumentada. [Avaliação](docs/foundation-001/evidence/sanitizer-evaluation.json). A revisão humana do ADR-001 deve considerar essa limitação.

## G. DataRoot

Exclusivo: `%LOCALAPPDATA%\Sentrium\VMS\dev\Foundation-001`, resolvido por SHGetKnownFolderPath. Diretórios ancestrais e arquivos são abertos com inspeção de reparse points e destino final por handle; arquivos com múltiplos hard links são rejeitados antes de truncar/gravar. Inputs não podem selecionar outro root. Runtime e testes possuem espaços separados; cada fixture tem UUID próprio.

I-3/I-4 demonstraram rejeição de traversal, UNC/device paths, nomes inseguros, junction, symlink e hard link reais, preservando o marker da fixture. O threat model exclui resistência absoluta contra outro processo privilegiado hostil alterando simultaneamente o filesystem. Nenhum DataRoot legado foi consultado. [Threat model](docs/foundation-001/evidence/threat-model.md) e [segurança/efeitos colaterais](docs/foundation-001/evidence/dataroot-security.md).

## H. Observabilidade

`sentrium-server --health` executa diagnóstico local e encerra. Saída JSON contém identidade, estado, DataRoot/recursos, uptime, startup/shutdown, workers, falhas de bootstrap e último erro. É um snapshot Ready obtido antes do shutdown, com contagem final de workers; não representa um processo persistente. Não expõe caminhos privados.

Logging JSONL com Timestamp UTC, Level, EventId, Component, Message, Version, BuildId e RunId. Até três arquivos de 16 KiB, com rotação circular simples. Métricas ainda indisponíveis são null. [Health real](docs/foundation-001/evidence/health-output.json), [exit code](docs/foundation-001/evidence/health-exit.txt) e [log completo](docs/foundation-001/evidence/health-runtime.jsonl).

## I. Critérios de aceite

A [matriz individual AC-01 a AC-26](docs/foundation-001/evidence/acceptance-matrix.md) contém, para cada critério, resultado, método, arquivo de evidência, observações e limitações.

| Critérios | Resultado |
|---|---|
| AC-01, AC-02, AC-03, AC-04, AC-05, AC-06, AC-07 | PASS |
| AC-08, AC-09, AC-10, AC-11, AC-12, AC-13, AC-14 | PASS |
| AC-15, AC-16, AC-17, AC-18, AC-19, AC-20, AC-21 | PASS |
| AC-22, AC-23, AC-24, AC-25, AC-26 | PASS |

Os resultados se aplicam ao escopo e threat model expressos na matriz. A ausência de transportes de rede e escritas indevidas foi avaliada pelos pontos de API do produto, importações, fixtures e artefatos; não se declara tracing global do Windows nem proteção contra adversário fora do modelo.

## J. ADRs

- [ADR-001 — EVIDENCE COLLECTED / REVIEW REQUIRED](docs/foundation-001/evidence/adr-001-evidence.md): C++20/x64, RAII, lifetime, cancelamento/shutdown demonstrados; execução ASan pendente por indisponibilidade.
- [ADR-002 — EVIDENCE COLLECTED / REVIEW REQUIRED](docs/foundation-001/evidence/adr-002-evidence.md): lock, ambiente, builds, testes, reprodução e SBOM registrados.

ADRs 001–010 permanecem **Proposed**, conforme a ordem de implementação. Nenhum arquivo de ADR foi criado para simular aceitação ou alterado para Accepted. Revisão humana necessária.

## K. Dependências e SBOM

[Inventário](docs/foundation-001/evidence/dependency-inventory.md) e [SBOM proporcional](docs/foundation-001/evidence/sbom-initial.md) distinguem ferramentas, bibliotecas próprias, runtime e transitivas observadas. Nenhuma biblioteca externa de produto, modelo ou SDK Intelbras. Importações reais foram examinadas, não apenas o manifest.

Versões/hashes dos runtimes presentes em [runtime-inventory.json](docs/foundation-001/evidence/runtime-inventory.json). CRT Debug local 14.44.35211.0 e CRT Release local 14.50.35719.0 são distintos do compilador aprovado; nenhum foi modificado. Dependências internas observadas do OS estão no dumpbin transitive inventory, sem alegação de inventário exaustivo de Windows.

Baseline vcpkg única em vcpkg.json, comparada com checkout e tag. Sem vcpkg-configuration.json. Licenças não verificadas são identificadas como NOASSERTION, sem suposições; CMake BSD de três cláusulas e registry vcpkg MIT foram conferidos nos arquivos locais.

## L. Rollback

**PASS** na demonstração isolada. Processo Release encerrou com código 0, workers=0 e evento shutdown-completed. Uma fixture foi arquivada do DataRoot exclusivo para a área de evidências após validação dos caminhos; hashes/tamanhos coincidiram e a origem deixou de existir. Zero processos Foundation remanescentes. [Procedimento](docs/foundation-001/evidence/rollback-report.md) e [evidência bruta](docs/foundation-001/evidence/rollback-result.json).

Não foi necessário desinstalar ferramentas globais. Os artefatos entregues e logs permanecem para auditoria; não foi executada remoção integral do projeto.

## M. Pendências e limitações

- Revisão humana das evidências e dos ADRs, que não foram aceitos automaticamente.
- ASan indisponível no toolchain instalado; nenhuma comprovação instrumentada de memória.
- Licenças não verificadas de ferramentas auxiliares/alguns binários documentadas no SBOM; direitos de distribuição não avaliados.
- Reprodução demonstrada para toolchain, fontes e resultados lógicos; não para igualdade byte a byte de PE/PDB.
- Proteção de paths conforme threat model aprovado; sem promessa contra adversário privilegiado concorrente.
- Sem tracing global de todas as operações internas do Windows; inventário transitivo do OS limitado e explicitado.
- Nenhuma falha obrigatória permanece aberta no escopo AC-01–AC-26. O histórico de falhas iniciais está preservado.

## N. Resultado final

**FOUNDATION-001 — PASS / READY FOR HUMAN REVIEW**

Verificação final: [final-verification.json](docs/foundation-001/evidence/final-verification.json) confirma as 14 entradas de fonte/build/scripts iguais às testadas, os 20 exit codes finais preservados e ausência de processos residuais. Nenhuma Foundation posterior foi iniciada, nenhum ADR foi aceito e nenhuma release comercial foi produzida.
