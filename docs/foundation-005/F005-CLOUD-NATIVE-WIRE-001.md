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
