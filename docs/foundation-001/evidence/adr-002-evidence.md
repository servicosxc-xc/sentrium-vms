# ADR-002 — EVIDENCE COLLECTED / REVIEW REQUIRED

Estado governado pela ordem R1: **Proposed**. Nenhum arquivo ADR foi alterado ou declarado Accepted.

Build Tools 17.14.41/build 17.14.37710.0; v143/família 14.44; VCToolsVersion 14.44.35207; cl FileVersion 19.44.35229.0; link FileVersion 14.44.35229.0. VsDevCmd oficial seleciona host/target x64 e SDK 10.0.26100.9457 (diretórios 10.0.26100.0). CMake 4.4.4 e Ninja 1.13.2 são endereçados por caminho. vcpkg registry tag 2026.10.16 e ferramenta 2026-09-26; baseline efetiva única em vcpkg.json, conferida contra HEAD e tag. Nenhum port foi instalado.

Evidências: preflight-report.md, preflight.json, vswhere.json, msvc-environment.txt, sdk-packages.json, toolchain-hashes.json; comandos/presets, build logs, JUnit e exit codes em sentrium-final e sentrium-reproduction; dependency-inventory.md e sbom-initial.md. O build reproduz os resultados lógicos com a mesma identidade de fonte; determinismo byte a byte dos binários não foi exigido nem demonstrado.

As ferramentas de desenvolvimento têm dependências próprias, que não se tornam bibliotecas do produto. A distinção é registrada no SBOM. Nenhuma revisão de ADR é automatizada.
