
## ADR-001 — linguagem-standard

**Status: Accepted**

**Contexto:** migração da r25, com paridade e rollback imutável; auditoria originalmente realizada sem implementação.

**Decisão originalmente proposta:** C++20 e RAII.

**Alternativas consideradas:** C++17 reduz recursos de concorrência; C++23 aumenta exigência do toolchain.

**Consequências:** exige ownership explícito, contratos testáveis, observabilidade e versões/licenças controladas. Não aprova a troca de produção nem elimina dependências de hardware/campo.

**Gate original para Accepted:** compilação MSVC x64, sanitizadores disponíveis, cancelamento/lifetime demonstrados.

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

- C++20 como padrão de linguagem.
- MSVC/v143 como toolchain Windows.
- RAII e ownership explícito.
- Cancelamento cooperativo.
- `std::jthread`.
- `std::stop_token`.
- Shutdown determinístico.

A aceitação foi baseada na revisão humana dos resultados da Foundation-001, incluindo compilação C++20 x64, testes de ownership/lifetime, cancelamento, shutdown e evidências documentadas.

### Evidência declarada

A Foundation-001 foi formalmente aprovada após revisão de:

- PRE-FLIGHT e toolchain.
- Builds Debug e Release.
- Reprodutibilidade.
- Testes unitários e de integração.
- Segundo ciclo independente de validação.
- Matriz AC-01 a AC-26.
- Evidências do ADR-001.

O registro de aprovação informa 26/26 critérios obrigatórios PASS.

Os logs, relatórios e resultados históricos originais permanecem como fontes de evidência; este ADR registra a decisão posterior e não substitui esses materiais.

### Limitação conhecida — ASan

O AddressSanitizer estava indisponível na instalação MSVC aprovada durante a Foundation-001.

Não houve execução do ASan e nenhum resultado de sanitização deve ser presumido.

O responsável avaliou expressamente essa limitação e determinou que ela **não impede a aceitação do ADR-001**.

Assim, a parte do gate histórico referente à disponibilidade de sanitizadores foi encerrada por **aceitação humana explícita da limitação**, não por comprovação técnica de execução.

### Validação futura desejável

Realizar sanitização instrumentada em ambiente compatível, preservando logs, resultados e eventuais limitações.

Essa atividade é recomendada e não reabre automaticamente a Foundation-001 nem modifica seu aceite histórico.

### Alcance da aceitação

Este ADR aceita a linguagem, os princípios de ownership/cancelamento e o toolchain Windows definidos na decisão.

Não aprova automaticamente:

- Frameworks de backend ou interface.
- SDKs Intelbras.
- FFmpeg.
- Decodificação GPU.
- ONNX Runtime.
- IA ou LPR.
- Distribuição comercial.
- Qualquer Foundation posterior.

**Decisão final: ADR-001 — ACCEPTED.**

---

**Integridade documental:** o contexto, as alternativas e as consequências da proposta original são preservados. A atualização formal consiste na mudança de status e no registro explícito da decisão humana e de sua exceção documentada.

---

