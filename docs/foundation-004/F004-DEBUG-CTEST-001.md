# F004-DEBUG-CTEST-001 — Build Debug e regressão (25/10/2026)

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** 004 — Media Lab  
**Branch:** `foundation/004-media-lab`  
**Estado da evidência:** DEBUG BUILD PASS / CTEST DEBUG PASS (33/33)  
**Fonte:** saída de PowerShell fornecida pelo operador em 25/10/2026. Logs originais de build permanecem no computador local.

## Configuração e compilação

- `git status --short`: sem alterações locais antes de atualizar.
- `git pull --ff-only origin foundation/004-media-lab`: fast-forward do commit `2312f31` para `3cb3c51`.
- `cmake --preset sentrium-debug`: PRE-FLIGHT PASS, configure PASS e generate PASS.
- `cmake --build --preset sentrium-debug`: compilação concluída com `[27/27] Linking CXX executable sentrium-client.exe`.
- Aviso `WrapVulkanHeaders` não encontrado: não bloqueou a construção; não requer mudança de toolchain a partir desta evidência.

## Testes Debug

Comando: `ctest --preset sentrium-debug --output-on-failure`.

| Grupo | Resultado |
|---|---|
| Unitários U-1 a U-8 | 8/8 PASS |
| Integração I-1 a I-11 | 11/11 PASS |
| Dispositivos D-1 a D-7 | 7/7 PASS |
| Mídia M-1 a M-6 | 6/6 PASS |
| F002-CLIENT-SMOKE | 1/1 PASS |
| **Total** | **33/33 PASS** |

- Falhas: **0**.
- Tempo total CTest: **15,17 s**.
- Smoke do cliente: **14,19 s**.
- `F002-CLIENT-SMOKE` é configurado para 10 ciclos no `CMakeLists.txt`; o script inclui verificação de processo `sentrium-client` residual por ciclo.

## Processo residual (consulta pontual)

Após os testes, o operador executou:

```powershell
Get-Process -Name "sentrium-client","sentrium-media-lab" -ErrorAction SilentlyContinue |
    Select-Object ProcessName, Id
```

A saída foi vazia: **nenhum processo desses nomes estava presente no instante da consulta**. Isso não é evidência de ciclos repetidos de início/encerramento do `sentrium-media-lab` em conexão real.

## Consolidação com Release

O documento `F004-RELEASE-CTEST-001.md` registra independentemente a execução Release, com **33/33 PASS** e 9,24 s. Ambas as configurações cumprem o gate automatizado de build/regressão, sem inferir validações de hardware adicionais.

## Limites e próximas evidências

- `F004-AC-01` (Debug build): PASS por evidência fornecida.
- `F004-AC-02` (Release build): PASS segundo relatório Release separado.
- `F004-AC-03` (regressão): PASS para suítes executadas em Debug e Release.
- `F004-AC-17` (zero processo residual): smoke do cliente e consulta pontual PASS; encerramento repetido do Media Lab **pendente**.
- Benchmarks em 4/16 câmeras reais: `NOT RUN` pela topologia previamente documentada.
- A consulta pontual não demonstra estabilidade de sessões NetSDK/PlaySDK em sucessivos ciclos, nem ausência de vazamento de handles/threads.
- ADR-005 permanece `Proposed`, Foundation-004 permanece `AUTHORIZED / IN IMPLEMENTATION`, sem merge na `main` e sem Human Review Approved.
