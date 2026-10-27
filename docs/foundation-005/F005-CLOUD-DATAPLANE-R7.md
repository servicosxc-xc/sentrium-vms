# SENTRIUM F005 R7: Integrated post-auth PTCP data plane

Date: 26/10/2026. Branch foundation/005-device-connectivity. PR #4 draft.

## Real baseline and fault fixed
Windows R5 passed on 97159ea2 (PRE-FLIGHT, Debug build, D-8/D-17/D-18 3/3). R6 build on c52f2e9 failed before tests with C4456 and C2220 from a shadowed id at main.cpp line 789. Fixed the loop name to messageIndex. No R6 test had run.

## Consolidated C++ implementation
cloud_ptcp_channel.hpp/.cpp implements PtcpPostAuthChannel, joining:
- outbound segmentation (up to 16 KiB/write, PTCP payload pieces <=1280 B), PID/LMID/accounting and admission backpressure before any partial send;
- receive order via PtcpReceiveWindow across heartbeat and payload, cumulative ACK validation via PtcpReliableSession, duplicate suppression and strict realm checks;
- explicit heartbeat, bounded exact-wire retries, proper expiry, DISC handling and fail-closed behavior for malformed frames or invalid ACK.

The provider MUST independently authenticate/pin a remote peer and derive realm and starting LMID from a legitimate handshake. A constructor or local fixture does NOT authorize Intelbras Cloud, start a socket or negotiate NAT. There are no third-party binaries, reverse-embedded vendor identities, SIM Next dependencies, or Defender exceptions.

## One Windows gate
CMake adds the R7 file and tests D-21..D-26; includes the pending R6 D-19/D-20. The single PowerShell runner builds Debug once and runs only D-19 through D-26. Qt/SDK roots are auto populated only when environment variables are absent and previously approved directories exist. Exact PRE-FLIGHT remains mandatory.
R6 fixed and R7 build/tests: NOT RUN after these commits. Do not invent a pass.

## Still missing for actual full Cloud-by-serial
- Documented, authorized application/bootstrap identity and API/SDK from the Cloud service owner.
- Authenticated rendezvous, NAT traversal, trusted Direct/Relay route telemetry, and verified remote session.
- An authorized Cloud provider to connect the live UDP protocol to this data plane, deliver a reliable stream to the loopback NetSDK bridge and manage reconnect/close.
- An end-to-end hardware test on an actual remote NVR 16/32 channel unit.

Intelbras technical support publicly distinguished ordinary IP/SDK access from a separate proprietary Cloud SDK in a 2018 forum statement. That historical statement does not establish availability in 2026 and should be re-checked with Intelbras. Separate Dahua development programs do not grant use of Intelbras Cloud by default.

An independent managed on-premise gateway+relay is another achievable product architecture for supported client locations, but requires mutual authentication, secure encryption, infrastructure and installation at the site. It is NOT in R7.

Git governance: PR #4 draft, no main merge, F004 Human Review remains pending, blocked dh-fwd remains quarantined.

Single next command in repository Developer PowerShell, SENTRIUM closed:
.\scripts\windows\sentrium-validate-f005-cloud.ps1


## Evidencia Windows superveniente (26/10/2026)

Operador confirmou em 2757ab6eab944b2c1953ad152532b0244e94c057: PRE-FLIGHT PASS, Debug build [44/44] PASS, D-19 ate D-26 8/8 PASS (0.13s). Esta informacao substitui o NOT RUN historico indicado antes. Os testes sao offline, nao Cloud remoto.
