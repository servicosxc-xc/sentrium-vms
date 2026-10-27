# F005 R9 — native pinned UDP socket

Date: 26/10/2026. Branch foundation/005-device-connectivity, PR #4 draft. R9 Windows build/tests NOT RUN.

## Baseline from actual operator log
Commit 1294f5c482071e41cb6c0adb3676ef2c7c8ed94b, PRE-FLIGHT PASS, focused sentrium-device-tests Debug build PASS, D-27..D-34 all 8/8 PASS in 0.09 seconds. The synthetic peer does not establish Intelbras Cloud.

## R9 features
New cloud_udp_socket.hpp/cpp exposes PinnedUdpSocket: real Windows Winsock UDP endpoint transport bound to caller-provided numeric IPv4 unicast and port, with OS-assigned local source port, exclusive bind, UDP connect and kernel source endpoint filtering; no DNS or invented endpoints. Native socket I/O enforces whole datagrams (up to 2048 bytes), bounded 0..1000 ms receive deadline, <=20 ms select slices, cooperative concurrent close, and fail-closed errors including truncated incoming frames. No external binaries or background processes.

**Security:** UDP connect endpoint filtering alone cannot cryptographically authenticate a Cloud peer. Deliberately PinnedUdpSocket does NOT implement VerifiedPtcpDatagramLink. A separately authorized provider must implement vendor bootstrap, peer authentication, session binding, Direct/Relay negotiation and any required data-plane integrity before connecting R9 I/O to R8. NEVER advertise this as operational Intelbras Cloud or promote a configured address to verified status.

D-35..D-40 use actual Windows UDP loopback sockets to test numeric IP validation, datagrams both directions, reject packets from other source ports, truncation fail-closed, cancellation with concurrent close, independent simultaneous connections and invalid API limits. These are LAN-local fixtures, not remote NVR or P2P tests.

CMake builds R9 with sentrium-device. The sole PowerShell runner preserves branch/clean worktree and exact SDK preflight, builds only target sentrium-device-tests, runs D-35..D-40, and reloads itself after Git pull if the script changed. No D-27..D-34 repeats. R9 still requires actual Windows validation.

## Unresolved vendor-side integration
Obtain legitimate Intelbras Cloud application identity and bootstrap API/SDK; verify authorized rendezvous and authenticated peer; measure NAT Direct/Relay route; create independently authorized and verified datagram link; then use R8 byte stream and existing TCP/NetSDK bridge for real 16/32-channel NVR. SIM Next and Defender-quarantined dh-fwd remain prohibited. No main merge or Human Review approval.

Single focal Windows command at repository root with client closed: scripts/windows/sentrium-validate-f005-cloud.ps1

## Verified operator Windows evidence — 27/10/2026

The operator submitted a Developer PowerShell log for commit 0f639c3e9638dcbd17850c0b118869032945fc2b: PRE-FLIGHT PASS, focused Debug device-tests build PASS, D-35 through D-40 **6/6 PASS in 0.26 seconds**. Script reloaded automatically after the Git pull. This supersedes historical R9 NOT RUN above. The actual Windows UDP tests are loopback and do not establish Intelbras Cloud.
