# F005-CLOUD-NATIVE-WIRE-001 — protocolo DH, parser C++ seguro

**Data:** 2026-10-26  
**Branch:** `foundation/005-device-connectivity`  
**Status:** IMPLEMENTED IN SOURCE / WINDOWS BUILD NOT RUN IN THIS CHECKPOINT  
**Cloud production:** BLOCKED — authenticated rendezvous/P2P/Relay still absent

## Why this increment

The operator confirmed a real 8-channel AutoRegistration connection. The previous third-party Cloud helper `dh-fwd_win_x64.exe` was quarantined by Microsoft Defender as `Trojan:Win32/Bearfoos.A!ml`. No bypass or re-execution is permitted. The goal is still **independent Cloud support in the native SENTRIUM C++ code**, while protecting the operational workstation and NVR credentials.

## Native implementation

Added:
- `libs/sentrium-device/include/sentrium/cloud_wire.hpp`
- `libs/sentrium-device/cloud_wire.cpp`

`parseCloudWireResponse` handles **DHP2P/1.0 signaling replies**, whose wire format is documented in independent research (see references). The parser has no network access and **no bundled Cloud client identifiers, bootstrap keys, hostnames, authentication secrets, external helper executables or SIM Next dependency**.

Behavior:
- Reads status (including authenticated errors 401/403 as valid protocol replies, never as connection success).
- Parses printable-ASCII headers, case-folds header names, rejects ambiguous duplicate fields.
- Accepts CRLF and LF newlines as observed in upstream protocol descriptions.
- Retains body bytes for a separate XML/protocol layer; it neither evaluates XML entities nor blindly trusts addresses contained in the body.
- Enforces IPv4 UDP maximum payload of 65,507 bytes, bounded total header section, maximum 64 headers, and validates Content-Length when supplied.
- Returns categorical errors instead of logging raw device IDs, cloud tokens, IP addresses or response bodies.
- Adds focused automated test `D-9` and includes the parser in the existing `sentrium-device` CMake target.

**Protocol references:** public read-only source inspected from `blackjid/dahua-p2p` `dh/protocol.go` / `dh/protocol_test.go` (MIT). The new SENTRIUM parser is independently authored, not a copied source file, and does not import the third-party module or any embedded app credentials. Public docs: `https://github.com/blackjid/dahua-p2p`.

## Defense in depth

`DeviceController::addCloudBridgeAndConnect` now refuses the external-bridge path **within C++**, even if invoked outside the disabled Qt controls. This prevents sending NetSDK credentials to arbitrary local processes while the Cloud pilot is under security hold. No fake tunnel, connected state or Direct/Relay indicator is introduced.

## What is not implemented yet

- Authenticated DH request generation using legitimate service-side credentials or an authorized service integration.
- Production UDP transport and bounded retries; trusted DH-server identity; signature/nonces; cloud rendezvous.
- Direct PTCP / Relay handshake, encrypted response handling and dynamic TCP local tunnel.
- NetSDK login through a live, independently validated Cloud tunnel.
- Clean Windows installer/distribution and permission to operate proprietary Cloud services.

These remain **BLOCKED / NOT RUN**, not PASS. Do not embed credentials extracted from SIM Next or other vendor apps. An MIT source code license is not a substitute for authorized access to the Cloud service.

## Validation gate

Since the parser adds a source file, Windows build requires CMake configure then build (once). Run **only** `D-9` as the focused offline parser regression; no 16/32 channel benchmarks or Cloud login test is implied.

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
. ".\scripts\windows\sentrium-preflight.ps1" -VerifyOnly
& $env:SENTRIUM_CMAKE --preset sentrium-debug
& $env:SENTRIUM_CMAKE --build --preset sentrium-debug
& (Join-Path (Split-Path $env:SENTRIUM_CMAKE -Parent) "ctest.exe") --preset sentrium-debug -R "^D-9$" --output-on-failure
```

Do not claim test success before the operator returns an actual toolchain log.

## Governance

F005 AUTHORIZED / IN IMPLEMENTATION. The branch is not merged. F004 and `main` remain untouched; ADR-005 Proposed; Human Review not approved.

## Lote maior de engenharia — transporte UDP e sinalização nativos (26/10/2026)

**Nova implementação integrada no mesmo branch, ainda SEM execução no Windows após este lote:**

- `cloud_udp.hpp` / `cloud_udp.cpp`: transporte Windows Winsock UDP síncrono para **um endpoint explícito IPv4 unicast**, recebido de camada superior legítima, sem DNS automático. Inicializa/encerra Winsock com RAII, fecha socket, fixa peer com `connect`, limita payload de saída a 8192 bytes e resposta a 65507 bytes, timeout configurável de 100 a 10000 ms, 1–3 tentativas e erros categorizados. Não invoca rede a menos que o chamador forneça request não vazia e configuração válida.
- `cloud_signaling.hpp` / `cloud_signaling.cpp`: compõe transporte UDP com `parseCloudWireResponse` do módulo anterior. Uma resposta `200` representa somente status do protocolo; `401/403/404` são rejeições, **não dispositivo conectado**. Dados da resposta são sensíveis e nunca devem ser registrados automaticamente.
- `CMakeLists.txt`: adiciona módulos ao target `sentrium-device` com link Winsock `ws2_32`.
- `tests/device/main.cpp`: **D-10** valida limites do transporte e falha fechada sem realizar tráfego de rede; **D-9** continua validando framing DHP2P.
- `scripts/windows/sentrium-validate-f005-cloud.ps1`: reúne git fast-forward, PRE-FLIGHT, configure/build e somente D-9/D-10 em **uma execução**, impedindo o linker de abrir `sentrium-client.exe` enquanto o programa está ativo.
- O usuário solicitou **entregas em lote e checkpoint somente ao final**, sem sucessão de pequenos comandos.

**O que isto NÃO é:** não possui provedor Cloud autenticado, chaves/IDs de aplicação, servidor bootstrap predefinido, negociação Direct/Relay, retransmissão PTCP nem túnel TCP para o NetSDK. Não chamar de módulo Cloud completo ou conexão real. As exigências de origem/segurança/autorizações permanecem. A F005 é IN IMPLEMENTATION.

**Último log trazido pelo usuário:** configure CMake PASS, build chegou ao LINK `sentrium-client.exe` e falhou **LNK1168 (arquivo aberto por processo)**. Não houve resultado D-9 nem D-10; ambos **NOT RUN** nesse log. O script novo impede esta classe de tentativa enquanto o cliente está aberto.

**Continuidade obrigatória:** `CONTINUE-HERE.md` na raiz e `docs/HANDOFF-SENTRIUM-CURRENT.md` guardam o estado factual e o prompt de transferência para outro chat. Atualizar a cada entrega significativa.
