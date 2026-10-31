# F005 R5: CloudStreamTunnel integration (26/10/2026)

## Baseline actually tested

Operator Windows log confirmed commit c30d25c65b0331dac0006c000712453e83edc84b: PRE-FLIGHT PASS; CMake configure/generate PASS; Debug build [44/44] PASS; D-15 and D-16 PASS, 2 of 2 in 0.08 seconds. This was a localhost mock echo and did not connect Intelbras Cloud. D-9 through D-14 passed on prior checkpoints. R5 code, build and new tests are NOT RUN on Windows.

## Current implementation (not a complete Cloud provider)

- Native CloudStreamTunnel in cloud_stream_tunnel.hpp/.cpp now adapts CloudLoopbackBridge to the CloudTunnel endpoint and close interface. A verified remote CloudByteStream, once independently furnished by an authorized provider, can be passed into this adapter so that the existing CloudDeviceSession passes a real ephemeral 127.0.0.1 TCP port to NetSDK.
- No CloudTransport implementation, vendor-app client identity, remote bootstrap host, rendezvous, actual Direct/Relay path or validated PTCP byte-stream reconstruction is included. Route always Unknown. Authentication is an essential PRECONDITION of the injected stream, not something the constructor magically enforces.
- CloudDeviceSession disconnect now retains its NetSDK session and tunnel if logout fails or throws; prevents destroying transport out from under the SDK. Next explicit disconnect may retry.
- D-17 uses local Windows TCP sockets through a fake provider and fake SDK-shaped Adapter to validate the complete logical orchestration (fixture channel counts 16 and 32, not hardware channels).
- D-18 tests a first failed logout and retry, absence of duplicate tunnel, null upstream rejection, invalid endpoint after closing, and Unknown route.
- Because cloud.cpp lifetime changed, the single Windows runner intentionally runs D-8 (related preexisting device test) along with new D-17/D-18; D-9..D-16 are not repeated.

## Gates and limitations

R5 remains source-reviewed only. Do not mark as compiled or passed until real Windows output is received. No NetSDK SDK was used by D-17/D-18, no 16/32-channel NVR was contacted, and no Cloud login was performed. The remote Cloud service still requires independently documented/authorized bootstrap, mutual peer validation, reliable byte ordering, and proper Direct/Relay telemetry. Security hold on dh-fwd remains; zero SIM Next dependency. PR #4 draft, main unchanged, Human Review pending.

Single focal command after pull/with SENTRIUM closed:
scripts/windows/sentrium-validate-f005-cloud.ps1


## R5 Windows validation confirmed 26/10/2026

The operator provided an actual PowerShell log for commit 97159ea2c0e0a274f68f5e7f4066c194a4462a6a. PRE-FLIGHT PASS, CMake configure/generate and Debug build PASS; D-8/D-17/D-18 all PASS (3/3, 0.10 seconds). Build output was reduced to the last few lines, so its exact [N/N] total is not known. This supersedes historical NOT RUN above. The synthetic 16/32 fixture counts were not real camera enumeration.
