# SENTRIUM VMS — Foundation-001

Foundation C++20 experimental `0.0.0-foundation001`, Windows x64. Escopo: core, ciclo de vida, DataRoot protegido, logging limitado e diagnóstico CLI local. Consulte [o resultado](SENTRIUM-CPP-Foundation-001-RESULTADO.md) e [a matriz de aceite](docs/foundation-001/evidence/acceptance-matrix.md).

## Build e testes

Na raiz autorizada, em PowerShell:

```powershell
./scripts/windows/sentrium-build.ps1 -RunName sentrium-minha-validacao
```

O script localiza o Build Tools aprovado com vswhere, inicializa VsDevCmd x64/x64 somente no processo, verifica todas as versões e a baseline única em vcpkg.json, compila Debug e Release do zero e executa CTest. Ferramentas portáveis são lidas de `../_toolchain`. Não são instalados ports ou ferramentas. As saídas padrão são `out/sentrium-debug` e `out/sentrium-release`.

O script recusa diretórios de build ou evidências já existentes. Para repetir, arquive os dois diretórios de build dentro de `out/`, após conferir os caminhos e encerrar os executáveis; use um RunName novo. Os builds desta entrega já estão presentes. Os logs completos, comandos estruturados, exit codes e resultados JUnit ficam em `docs/foundation-001/evidence/<RunName>/`.

## Diagnóstico local

```powershell
./out/sentrium-release/sentrium-server.exe --health
./out/sentrium-release/sentrium-server.exe --version
```

`--health` inicializa os recursos mínimos, registra o diagnóstico, cancela e aguarda o worker, finaliza o logging e encerra. A saída JSON descreve o diagnóstico `Ready` calculado antes do shutdown, com a contagem final de workers e duração do shutdown. Não é o estado de outro processo. Exit codes: 0 sucesso, 2 configuração, 3 filesystem/lock, 4 recurso, 5 erro interno. Não há modo servidor persistente.

DataRoot resolvido pela Known Folder API: `%LOCALAPPDATA%/Sentrium/VMS/dev/Foundation-001`. Logs em `runtime/logs`, até três slots de 16 KiB. A execução concorrente no mesmo runtime é rejeitada por lock exclusivo. A opção interna `--test-run <UUID>` isola as fixtures em `tests/<UUID>/runtime`; ela aceita somente um identificador, nunca um caminho ou override do DataRoot.

## Limites

As proteções rejeitam reparse points, caminhos não canônicos, destinos por handle divergentes e arquivos com múltiplos hard links. O threat model não cobre resistência absoluta contra outro processo privilegiado hostil alterando simultaneamente o filesystem. Testes de symlink dependem da capacidade já concedida pelo Windows; não habilitam privilégios. O produto não solicita elevação.

ASan foi avaliado, mas suas bibliotecas/runtime não estão instaladas no toolchain aprovado. A configuração `sentrium-asan` é opcional e não foi executada nesta entrega. ADRs continuam Proposed; esta entrega não constitui release comercial.
