# SENTRIUM — SBOM inicial proporcional

Produto SENTRIUM VMS; Foundation-001; `0.0.0-foundation001`; build `source-b8b6d42374064dc8`; Windows x64; MSVC C++20. Identidade sem Git, derivada da lista de hashes em source-inventory.json. Hashes de executáveis Debug/Release ficam em `sentrium-final/*-binary-hashes.json` e `sentrium-reproduction/*-binary-hashes.json`.

## Ferramentas de desenvolvimento

| Componente | Versão/origem | Licença conhecida / limite |
|---|---|---|
| Build Tools / MSVC | Microsoft, 17.14.41 / build 17.14.37710.0; v143, VCToolsVersion 14.44.35207, cl 19.44.35229.0, link 14.44.35229.0 | Termos Microsoft; direitos de redistribuição não avaliados |
| Windows SDK | Microsoft, produto 10.0.26100.9457, pacotes MSI 10.1.26100.9457, dirs 10.0.26100.0 | Termos Microsoft; direitos de redistribuição não avaliados |
| CMake | Kitware, distribuição local 4.4.4 | BSD de três cláusulas, verificado em doc/cmake/LICENSE.rst; componentes agregados possuem avisos próprios |
| Ninja | Distribuição local ninja-1.13.2 | NOASSERTION: arquivo de licença não presente no diretório local; não inferida |
| vcpkg registry | microsoft/vcpkg, tag 2026.10.16; commit conforme builtin-baseline em vcpkg.json | MIT, verificada no LICENSE.txt do checkout |
| vcpkg-tool | Microsoft, 2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b | NOASSERTION para o binário isolado; checkout e ferramenta são inventariados separadamente |
| Git / PowerShell | Ferramentas locais auxiliares; versões em auxiliary-tool-inventory.json | Licenças não verificadas nesta Foundation |

Caminhos, versões efetivas e SHA-256 das seis ferramentas principais estão em toolchain-hashes.json e preflight.json. A origem de download detalhada preexistente está no relatório de ambiente em `../_toolchain/SENTRIUM-DEVELOPMENT-ENVIRONMENT-RESULTADO.md`; esta Foundation não fez downloads. Componentes internos do CMake identificáveis pelos avisos locais são inventariados em tool-license-inventory.json. Não foi criada uma árvore exaustiva dos payloads dos instaladores Microsoft.

## Componentes do binário e runtime

Diretos próprios: sentrium-server, sentrium-platform-win e sentrium-core. Licença comercial do produto ainda não declarada; não foi inventada uma licença. Bibliotecas de terceiros de produto: **nenhuma**. Runtime necessário: APIs Windows, UCRT e MSVC CRT/STL conforme as tabelas de dependency-inventory.md. Debug depende de runtime de desenvolvimento; não é artefato de distribuição.

Transitivas observadas dos componentes runtime: nomes brutos em runtime-transitive-dependents.txt. Versões/hashes dos componentes primários presentes em runtime-inventory.json. Os contratos API-set são resolvidos pelo Windows; não são inventados hashes de DLLs físicas correspondentes. Componentes Windows/MSVC têm termos Microsoft, sem avaliação de redistribuição nesta entrega.

O inventário é proporcional ao executável mínimo: não há dependências de ports, modelos, SDKs proprietários de vídeo, mídia ou UI. Lacunas explícitas de licença e de inventário interno do OS permanecem documentadas para revisão, sem atribuição por suposição.
