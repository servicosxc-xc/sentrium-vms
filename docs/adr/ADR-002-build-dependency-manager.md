
## ADR-002 — build-dependency-manager

**Status: Accepted**

**Contexto:** migração da r25, com paridade e rollback imutável; auditoria originalmente realizada sem implementação.

**Decisão originalmente proposta:** CMake + Ninja + vcpkg manifest/baseline.

**Alternativas consideradas:** MSBuild puro ou Conan.

**Consequências:** exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

**Gate original para Accepted:** reprodutibilidade Debug/Release, dependências e SBOM fixados.

**Referências históricas:** [arquitetura](../ARQUITETURA-CPP-PROPOSTA.md), [riscos](../RISCOS-E-BLOQUEIOS.md), [licenças](../DEPENDENCIAS-E-LICENCAS.md), [roadmap](../ROADMAP-CPP.md).

---

## Registro de aceitação — Human Review da Foundation-001

**Estado anterior:** Proposed  
**Novo estado:** Accepted  
**Autoridade:** decisão explícita do responsável pelo projeto  
**Fundamentação:** SENTRIUM-CPP-Foundation-001 — PASS / HUMAN REVIEW APPROVED  
**Foundation associada:** SENTRIUM-CPP-Foundation-001  
**Decision Gate:** F001 — CLOSED SUCCESSFULLY

### Decisão aceita

Adotar:

- CMake.
- Ninja.
- vcpkg em modo manifest.
- Baseline/commit de dependências fixado.
- Presets Debug e Release.
- Builds isolados por configuração.
- Automação reproduzível de configuração, compilação e testes.
- Inventário de dependências.
- SBOM proporcional ao escopo.

### Identidade do toolchain validado

A Foundation-001 utilizou a baseline aprovada no SCOPE 1.1:

| Componente | Identificador |
|---|---|
| Visual Studio 2022 Build Tools | 17.14.41 |
| Build | 17.14.37710.0 |
| Platform Toolset | v143 |
| Família MSVC | 14.44 |
| VCToolsVersion | 14.44.35207 |
| cl.exe | 19.44.35229.0 |
| link.exe | 14.44.35229.0 |
| Windows SDK | 10.0.26100.9457 |
| CMake | 4.4.4 |
| Ninja | 1.13.2 |
| vcpkg registry/tag | 2026.10.16 |
| vcpkg commit | 9eec002df1cd109f8f104cd966a763daf9751489 |
| vcpkg-tool | 2026-09-26 |
| Host / target | x64 / x64 |

Os identificadores acima documentam a baseline do experimento aprovado.

A aceitação do ADR-002 não exige que todo desenvolvimento futuro permaneça indefinidamente nessas versões exatas. Alterações futuras de toolchain deverão seguir controle de versões, compatibilidade, rastreabilidade e aprovação aplicável.

### Evidência declarada

A revisão humana confirmou:

- PRE-FLIGHT aprovado.
- Builds Debug e Release.
- Reprodução dos builds.
- 19 testes unitários/de integração por configuração.
- Segundo ciclo independente de validação.
- Inventário de dependências.
- SBOM proporcional.
- Evidências documentadas do ADR-002.
- Matriz AC-01 a AC-26 integralmente PASS.

A presente atualização registra a aceitação e não reexecuta nem reescreve esses resultados.

### Pendências não bloqueantes

**Política MSVC/CRT:** definir, antes da distribuição, a estratégia de runtime, implantação e redistribuição correspondente.

**Identidade Git:** criar e formalizar o repositório do SENTRIUM antes do desenvolvimento funcional relevante. O identificador primário de builds futuros deverá utilizar a identidade do commit quando disponível, preservando o identificador histórico da Foundation-001.

**SBOM:** manter sua evolução proporcional às dependências efetivamente incorporadas, sem tratar a SBOM inicial como homologação comercial.

### Alcance da aceitação

O ADR aceita o sistema de build e o gerenciamento de dependências definidos.

Não aprova automaticamente frameworks, bibliotecas, licenças comerciais ou artefatos de distribuição não avaliados.

**Decisão final: ADR-002 — ACCEPTED.**

---

**Integridade documental:** a proposta e os critérios originais permanecem identificáveis. A mudança formal é o status Accepted e o registro da revisão humana que fundamenta a decisão.

---

