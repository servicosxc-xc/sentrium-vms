# F005 R4 — Native loopback TCP bridge

Date: 26/10/2026. PR #4 draft; main unchanged. **R4 Windows build and D-15/D-16: NOT RUN.**

## Known verified evidence
Operator's Windows log on commit eb7317c9ce7b088d58cda2c3b883b3ced0dda8fa: PRE-FLIGHT PASS, Debug build [48/48] PASS, D-11 through D-14 all PASS (4/4), nonblocking WrapVulkanHeaders warning. D-9/D-10 were passed earlier. These are offline/cryptographic/serialization tests, NOT a real authenticated Cloud session.

## Implementation
Two independently authored C++20 files in sentrium-device:
- cloud_loopback_bridge.hpp/.cpp: a Windows WinSock TCP listener bound ONLY to 127.0.0.1, on an OS-assigned ephemeral port; accepts one local TCP client and forwards bounded 8KiB byte chunks in-order to an injected CloudByteStream. No public bind, port forward, endpoint discovery, hard-coded service port or hidden process.
- CloudByteStream: upstream must have already completed independent authorized application bootstrap and peer authentication. It must honor short read deadlines and unblock on close. Inbound/outbound payloads can contain recorder passwords: never log them.
- Bridge teardown is cooperative and joins its worker; caller must log out NetSDK before closing a real provider's tunnel. No DeviceController/CloudTransport integration yet, and an in-memory echo is strictly a test fixture.

## Focal Windows validation
CMake adds bridge source and D-15/D-16; D-15 tests null stream rejection, dynamic local port and idempotent close, D-16 checks short TCP roundtrip through local fake echo stream using real Windows loopback sockets. No Cloud server is contacted. scripts/windows/sentrium-validate-f005-cloud.ps1 builds Debug and only executes D-15/D-16, avoiding retesting already-approved D-9 through D-14.

## Remaining blockers
Cloud application/bootstrap contract must be legitimately authorized and validated; device handshake/nonce proof is not equivalent to infrastructure authorization. Add peer-pinned Direct/Relay rendezvous with observed route telemetry, PTCP stream reordering/recovery, then supply an authenticated CloudByteStream to this loopback primitive, connect NetSDK and enumerate 16/32 channels with recorder under operator control. Security hold remains for Defender-quarantined dh-fwd; no SIM Next dependencies.


## Windows evidence updated 26/10/2026

Operator confirmed R4 build [44/44] PASS and D-15/D-16 both PASS on commit c30d25c65b0331dac0006c000712453e83edc84b. The heading above saying NOT RUN is the historical pre-validation record; this newer section supersedes it. Current R5 work is separate, awaiting validation.
