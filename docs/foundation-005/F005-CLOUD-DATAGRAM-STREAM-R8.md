# F005 R8 — Verified PTCP datagram to TCP byte stream

Date: 26/10/2026. PR #4 remains draft. Main and F004 Human Review unchanged.

## Established baseline

Windows R7 at commit 2757ab6eab944b2c1953ad152532b0244e94c057: PRE-FLIGHT PASS, Debug build [44/44] PASS and D-19 to D-26 8/8 PASS in 0.13 s. WrapVulkanHeaders was a nonblocking warning. The R7 fixtures are offline PTCP, not Intelbras Cloud.

## Consolidated R8 implementation

New libs/sentrium-device/cloud_ptcp_stream.cpp and include/sentrium/cloud_ptcp_stream.hpp define PtcpVerifiedByteStream : CloudByteStream and its injected VerifiedPtcpDatagramLink contract.

- Convert up to 16KiB full local writes to bounded PTCP datagrams and 128KiB bounded incoming ordered byte queue through PtcpPostAuthChannel.
- Schedule monotonic exact-wire retransmissions, pump verified incoming ACKs when bounded PTCP window is full; never silently drop NetSDK writes. On partial datagram transmission, fail closed rather than falsely report success.
- Deny invalid peer ACK/protocol or remote disconnect and close the link. Allow close from another thread while read/write run on one bridge worker.
- No Cloud bootstrap, vendor app identity, UDP socket, NAT traversal, Direct/Relay or real network transport is instantiated. A name containing Verified is NOT an authentication guarantee: the link implementation must authenticate and pin the peer externally and must unblock on close.

D-27..D-32 run synthetic whole-datagram echo/fragmentation, out-of-order and duplicate delivery, partial send loss, forged ACK, backpressure recovery with ACK and a real local OS TCP socket through CloudLoopbackBridge to the *synthetic* PTCP link. No remote NVR, no real SDK login or 16/32 channel enumeration is involved.

CMake includes R8 source and registers D-27 through D-32. The single existing PowerShell script builds Debug and runs only these new tests. R8 Windows build/tests: NOT RUN until the operator reports them.

## Critical remaining integration dependency

To provide actual Intelbras Cloud by device serial:
1. Receive legitimate authorization and bootstrap/application identity protocol from the service owner; never extract/repurpose secrets from SIM Next.
2. Establish authenticated rendezvous, peer identity, Direct/Relay routing with observed proof, and vendor-compatible session lifecycle.
3. Implement a concrete peer-pinned VerifiedPtcpDatagramLink, connect it to R8, provide the dynamic TCP port to real NetSDK, verify remote recorder channels and reconnect semantics.
4. Validate NVRs with 16/32 channels using a test site with permission.

If official Cloud integration is unavailable, customer-managed secure site gateway with an encrypted, mutually authenticated relay is a different architecture requiring actual remote installation; not implemented here. Keep dh-fwd quarantined and Cloud UI fail-closed.

Windows command, SENTRIUM closed, at the repository root:
.\scripts\windows\sentrium-validate-f005-cloud.ps1


## Final R8 pre-build hardening

Callbacks CloudByteStream are noexcept, so exceptions from allocations in queues/counters cannot propagate to NetSDK or call std::terminate. Internal retry, input pump and write now catch unexpected exceptions, tear the stream down and report failure. Additional tests D-33 and D-34 cover invalid arguments, overlong TCP writes and an oversized incoming datagram. One Windows runner now selects D-27 through D-34. All R8 tests remain NOT RUN until actual Windows evidence; do not inherit the R7 PASS.


## Focused build gate

R8 runner now targets CMake sentrium-device-tests and the dependent native sentrium-device library only, rather than unrelated Qt client/media/server. It still configures the same debug preset, verifies exact toolchain and runs D-27..D-34. It does NOT certify a full-client build, which will occur at a later integration gate. R8 Windows NOT RUN at this writing.
