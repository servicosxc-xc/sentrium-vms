# Foundation-005 R6 — Ordered PTCP packet receive window

Date 26/10/2026. Branch foundation/005-device-connectivity, PR #4 draft. R6 Windows build/D-19/D-20: NOT RUN.

## Real verified baseline

Operator confirmed the R5 commit 97159ea2c0e0a274f68f5e7f4066c194a4462a6a with PRE-FLIGHT PASS, CMake configure/build Debug PASS and D-8/D-17/D-18 3/3 PASS in 0.10s. Prior D-9..D-16 previously passed. The build step counter was not fully visible in the R5 log. None of these checks prove Cloud bootstrap or connection to a real remote NVR.

## Code changes

New portable C++20 PtcpReceiveWindow (cloud_ptcp_receive_window.hpp/.cpp). Requires an externally verified initial LMID and authenticated/pinned input peer. Preserves order across PTCP packet kinds, including payload and heartbeat/control, instead of assuming every LMID is data. Queues a maximum of 32 out-of-order packets or 256 KiB, rejects overlarge packet, gap beyond 32 message IDs, conflicting replay and buffer exhaustion. Fails closed and clears queued memory on fatal errors. Uses unsigned modular LMID differences to handle 32-bit wrap, then exact ID lookup for contiguous drain. Duplicate/old packets are not emitted twice.

This queue does not authenticate the source or perform ACK, timeout, retransmission, NAT traversal, reassembly into a NetSDK TCP session or vendor application bootstrap. It is not connected to DeviceController or a production CloudByteStream. Do NOT show the route as Direct or Relay.

## Focused tests

D-19: packets received out of order, duplicate suppression, heartbeat interleaving with payload, LMID contiguous release.
D-20: LMID wraparound, conflicting replay, excessive gap and memory bound fail closed.

CMake now includes R6 and declares D-19/D-20. The sole Windows validation script builds Debug and runs only D-19/D-20. Older approved D-8..D-18 are not rerun.

## Real Cloud blocker

Permissioned identity/credentials of the remote infrastructure remain unavailable, so we cannot claim or test Cloud authentication, remote peer negotiation, direct/relay routing, PTCP over a real network, NetSDK through an authorized tunnel or remote NVR with 16/32 channels. No SIM Next DLL, vendor bootstrap secret, fixed address or defender-quarantined executable is part of this implementation. Next permitted step after R6 is an authorized provider and live hardware validation, NOT an invented endpoint.


## R6 Windows result and corrective continuation (26/10/2026)

R6 operator log: MSVC compilation FAIL before CTest. Warning C4456 shadowed id loop variable in tests/device/main.cpp line 789; /WX promoted to C2220, exit 2. The loop variable was renamed messageIndex in consolidated R7. D-19 and D-20 remain NOT RUN pending the combined R7 single gate.
