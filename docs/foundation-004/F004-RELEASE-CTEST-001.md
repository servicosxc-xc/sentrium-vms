# F004 — Release Build and CTest regression (25/10/2026)

**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Branch:** `foundation/004-media-lab`  
**Estado:** **RELEASE BUILD PASS / CTEST RELEASE PASS (33/33)**  
**Proveniência:** Saída de PowerShell fornecida pelo operador; logs completos permanecem no ambiente local e não foram copiados ao GitHub.

## Ambiente e configuração

- Atualização Git fast-forward para `2312f3160c919aa7f2761e730a0bc0db4564a144`.
- PRE-FLIGHT da Foundation-004: PASS; toolchain exato, Qt 6.12.0 MSVC x64, NetSDK e PlaySDK externos, baseline vcpkg preservada.
- `cmake --preset sentrium-release`: configure/generate concluídos.
- `cmake --build --preset sentrium-release`: concluído com `[49/49] Linking CXX executable sentrium-client.exe`.
- Aviso `WrapVulkanHeaders` não encontrado: não bloqueou configure/build. Nenhuma instalação autorizada/necessária por este resultado.

## Regressão automatizada Release

Comando executado pelo operador:

```powershell
& (Join-Path (Split-Path $env:SENTRIUM_CMAKE -Parent) "ctest.exe") `
    --preset sentrium-release --output-on-failure
```

| Suíte | Resultado |
|---|---|
| U-1 a U-8 | 8/8 PASS |
| I-1 a I-11 | 11/11 PASS |
| D-1 a D-7 | 7/7 PASS |
| M-1 a M-6 | 6/6 PASS |
| F002-CLIENT-SMOKE | 1/1 PASS |
| **Total** | **33/33 PASS** |

- Testes falhos: **0**.
- Duração total CTest: **9,24 s**.
- Smoke cliente: **8,33 s**. CMake registra `-Cycles 10`; script confirma encerramento normal e ausência de `sentrium-client` residual em cada ciclo.

## Limites da evidência

- Confirma build Release e regressão automatizada no ambiente testado.
- Testes automáticos não substituem as validações reais de processo `sentrium-media-lab`, múltiplos streams, CPU/GPU e caminho de render.
- Não há evidência nesta saída de Release executando stream real ou PlaySDK pelo laboratório.
- Não houve merge, Human Review, mudança de ADR nem fechamento de F004.

**Estado:** F004 IN IMPLEMENTATION; F004-AC-02 PASS e regressão CTest Release observada; F004-AC-17 não foi integralmente fechado para o laboratório de mídia.
