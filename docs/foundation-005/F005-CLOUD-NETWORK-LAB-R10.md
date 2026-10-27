# F005 R10 — real OS TCP → PTCP → UDP integration laboratory

Date: 27/10/2026. Branch foundation/005-device-connectivity, PR #4 draft. **R10 Windows build/tests: NOT RUN.**

## Proven R9 baseline

Actual operator Windows PowerShell log on commit 0f639c3e9638dcbd17850c0b118869032945fc2b: PRE-FLIGHT PASS, focused Debug build PASS and D-35..D-40 all **6/6 PASS** in 0.26 seconds. The runner successfully reloaded the new script after fast-forward. This validates local Winsock UDP primitives only.

## New consolidated network integration tests

Rather than add another production class with an unverified vendor Cloud bootstrap, R10 adds a **single test-only executable** `sentrium-cloud-network-tests` built from `tests/device/cloud_network_lab.cpp`. Tests exercise actual Windows loopback UDP sockets to cross previously isolated production modules.

The `LabPeer` is a locally controlled UDP responder that receives framed PTCP Payloads and emits synthetic LMID/ACK data. It can drop a packet, reply with a forbidden realm or forged cumulative ACK, or remain silent. `UnverifiedLabLink` wraps the production `PinnedUdpSocket` in the injected R8 datagram interface **only inside this test source**. The name highlights that this fixture is NOT authenticated. Never compile or register it in the production Cloud provider.

D-41: 3000 bytes segmented and echoed over real OS UDP via production PTCP R7/R8/R9.

D-42: 3000 bytes from a real TCP socket through existing CloudLoopbackBridge, PtcpVerifiedByteStream, PinnedUdpSocket and local UDP responder, back to TCP in original order. This is NOT a NetSDK SDK login.

D-43: PTCP forged datagram from a different UDP port filtered by connected UDP kernel pin; legitimate local peer remains accessible. IP/port filtering is not cryptographic authentication.

D-44: wrong realm and forged future ACK fail-closed on real UDP receive.

D-45: first datagram dropped intentionally, exact PTCP retry recovers via real UDP.

D-46: concurrent stream shutdown interrupts an active UDP read and refuses further writes.

CMake registers D-41..D-46 on a dedicated `sentrium-cloud-network-tests` executable compiled with /W4 /WX. Existing one-command PowerShell runner builds this target (and linked native sentrium-device) with the locked Debug toolchain, then runs only these six tests. Other approved tests are not repeated.

## Vendor dependencies, no false Cloud claim

No proprietary Cloud bootstrap identity, authorized rendezvous, peer authentication, session-key derivation, NAT/Direct/Relay routing or real NVR is used by this lab. All network packets stay on 127.0.0.1. Even a successful R10 **cannot** prove 16/32 channels remotely or validate vendor serial Cloud. The next decisive work after this gate requires official Intelbras Cloud authorization/SDK OR a separately scoped secure managed gateway for client-controlled sites, not more mocked P2P handshakes.

No SIM Next artifacts, no Defender-quarantined dh-fwd, no embedded endpoints or static vendor credentials. PR draft, main untouched, no Human Review approval.

Single Windows command with client closed at repository root:
`\.\scripts\windows\sentrium-validate-f005-cloud.ps1`


## Operator Windows checkpoint 27/10/2026

Actual log on commit c211b964eefea6caa9f74c88bba9095ef9984e6d: PRE-FLIGHT, CMake and Debug sentrium-cloud-network-tests build [8/8] PASS; D-41 through D-46 6/6 PASS in 1.38 s. This supersedes earlier NOT RUN, proves local OS TCP and UDP with synthetic responder only, not authenticated vendor Cloud. Next action is supplier authorization and supported Cloud API contract, not more synthetic protocol simulations. See F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md.
