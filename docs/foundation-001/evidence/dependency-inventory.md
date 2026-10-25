# SENTRIUM — dependências efetivas

Nenhuma biblioteca externa de produto foi instalada, incorporada ou ligada. O manifest tem dependencies vazio. `sentrium-core` e `sentrium-platform-win` são bibliotecas próprias estáticas. A biblioteca padrão, CRT e APIs Windows são dependências normais documentadas.

| Configuração | Importações diretas observadas no executável |
|---|---|
| Debug | SHELL32.dll, ole32.dll, KERNEL32.dll, MSVCP140D.dll, MSVCP140D_ATOMIC_WAIT.dll, VCRUNTIME140D.dll, VCRUNTIME140_1D.dll, ucrtbased.dll |
| Release | SHELL32.dll, ole32.dll, KERNEL32.dll, MSVCP140.dll, MSVCP140_ATOMIC_WAIT.dll, VCRUNTIME140.dll, VCRUNTIME140_1.dll; contratos api-ms-win-crt heap/string/runtime/stdio/time/math/locale |

I-9 inspeciona o PE real e rejeita dependências fora dos nomes Windows/MSVC permitidos. dumpbin /imports preserva também os nomes das funções. Nenhuma importação de SIM Next, NetSDK, PlaySDK, libt2u, FFmpeg, Qt, Drogon ou ONNX Runtime. O código não carrega DLLs/modelos dinamicamente, não implementa API/listener e não contém código de mídia/IA/LPR.

`runtime-inventory.json` registra caminhos candidatos do runtime presente, versões e hashes. Debug CRT local: 14.44.35211.0; Release CRT local: 14.50.35719.0. São runtimes já instalados no sistema, distintos da versão do compilador fixado em 19.44.35229.0. Nenhum runtime foi substituído por esta execução. A ordem R1 não fixa uma revisão do runtime Release do Windows, e esses valores não foram usados como substituição do toolset.

`runtime-transitive-dependents.txt` registra dependências de primeiro nível dos componentes runtime/OS inventariados, inclusive contratos API-set e componentes Windows usados internamente por Shell/OLE. Capacidades internas desses componentes do OS não significam integração implementada pelo produto. Não se declara uma árvore exaustiva de todas as dependências internas do Windows nem rastreamento dinâmico de módulos carregados. A resolução System32/PATH foi inventariada como candidata, sem alegar tracing.

As dependências do processo de build (CMake/Ninja/vcpkg/Git/PowerShell) ficam separadas no SBOM. vcpkg é verificado apenas como baseline do ambiente; não há toolchain vcpkg integrado nem instalação automática.
