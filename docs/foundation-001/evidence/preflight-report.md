# SENTRIUM Foundation-001 — PRE-FLIGHT R1

Data: 2026-10-24 (America/Sao_Paulo). Resultado inicial: **PREFLIGHT PASS**.

Inspeção exclusivamente de leitura, antes da criação dos arquivos do projeto. Raiz autorizada vazia: `C:\Projetos\SENTRIUM VMS\SENTRIUM-CPP-Foundation-001`. Nenhum checkout Git no workspace ou nos seus ancestrais; nenhum commit inventado. Raiz e ancestrais inspecionados sem reparse points. Não houve acesso a dados legados.

| Requisito | Evidência detectada | Resultado |
|---|---|---|
| Windows | NT 10.0.26200.0 | PASS |
| Build Tools 2022 | vswhere: 17.14.41 / 17.14.37710.0, instância 563a9b85, completa e inicializável | PASS |
| Instalação | `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` | PASS |
| Toolset | v143 presente; VC.Tools.x86.x64 confirmado por vswhere -requires | PASS |
| VCToolsVersion | 14.44.35207 | PASS |
| cl.exe | FileVersion 19.44.35229.0, Hostx64/x64 da instalação aprovada, PE 8664 | PASS |
| link.exe | FileVersion 14.44.35229.0, Hostx64/x64 da instalação aprovada, PE 8664 | PASS |
| Ambiente | VsDevCmd oficial; VSCMD_ARG_HOST_ARCH=x64, VSCMD_ARG_TGT_ARCH=x64 | PASS |
| SDK | Pacote Windows 10.0.26100.9457; MSI 10.1.26100.9457; rc ProductVersion 10.0.26100.9457 | PASS |
| Headers/libs SDK | Include/Lib 10.0.26100.0; Windows.h e kernel32.lib x64 presentes; INCLUDE/LIB oficiais selecionam esses diretórios | PASS |
| CMake | `_toolchain/cmake-4.4.4/bin/cmake.exe --version`: 4.4.4 | PASS |
| Ninja | `_toolchain/ninja-1.13.2/ninja.exe --version`: 1.13.2 | PASS |
| vcpkg | HEAD e tag 2026.10.16 iguais ao commit aprovado na ordem R1; remoto microsoft/vcpkg; árvore limpa | PASS |
| vcpkg-tool | 2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b | PASS |
| Capacidade C++20 x64 | Compilador/linker AMD64, libs x64; evidência preexistente smoke-corrected-lock/result.json registra build e execução C++20 x64 bem-sucedidos | PASS |

Distinção: a revisão de servicing do SDK é 9457, enquanto seus diretórios permanecem 10.0.26100.0. Não se exige igualdade entre a versão MSI, a versão do produto e a versão dos diretórios. Também se distinguem VCToolsVersion e FileVersion do compilador/linker.

Nenhuma compilação foi executada durante o PRE-FLIGHT. A evidência de smoke test preexistente foi consultada somente para complementar a inspeção do ambiente, sem alterar `_toolchain`. Os builds próprios e a verificação dos binários da Foundation serão registrados na Fase 5.

O script sentrium-preflight.ps1 repete os checks antes de cada build e exporta os resultados brutos no diretório da execução. O valor efetivo da baseline vcpkg reside somente em vcpkg.json; não há configuração de registry adicional, integração do toolchain vcpkg nem instalação de ports. A Foundation utiliza apenas a biblioteca padrão e APIs Windows.
