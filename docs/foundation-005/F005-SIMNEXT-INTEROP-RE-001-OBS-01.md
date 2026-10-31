# RE-001 Observation 01 — SIM Next local TCP/UDP metadata

27 October 2026. User-provided JSON captured by the authorized, passive Windows observer; 25.2 seconds, label Connecting. No payload or DNS records, no process memory, no real cloud login test.

## Verified measurements

- One process: SIMNext.exe (PID 22076), version 1.25.4.1203, code signature Valid. 196 loaded library NAMES, including avnetsdk.dll, dhnetsdk.dll, dhconfigsdk.dll, dhplay.dll and StreamConvertor.dll. Presence is not proof which module owns a P2P handshake.
- 411 grouped network state observations: 322 TCP and 89 UDP local endpoints. These are NOT 411 concurrent Cloud connections.
- Among TCP entries: 161 local wildcard IPv4 (0.0.0.0), 1 wildcard IPv6 (::), 146 localhost (127.0.0.1), 14 non-local TCP entries.
- 144 localhost Established entries correspond to 72 reciprocal connected pairs. May be internal IPC/proxies; not evidence of an authenticated Cloud tunnel, device count, or per-camera association.
- UDP: 89 distinct local source ports; 75 first seen in first five seconds, 14 subsequently. The Windows endpoint cmdlet cannot report remote UDP peers, packet payload or route.
- Port 37000/TCP: three SynSent attempts ~0-6.3s, 7.4-16.2s and 17.3-24.1s.
- Port 37777/TCP: one preexisting Established connection, plus SynSent observed ~11.8-20.8s and ~21.9-24.1s. Other remote service ports included 37001, 37719, 443, 7000, 12246.
- Some remote 443 transitioned Established to CloseWait after ~10.7s.
- Approximately 22 distinct observation times during 25.2s despite configured interval 500ms, because querying the Windows TCP and UDP tables adds overhead. firstSeen is first sampled, not exact opening time.

## Interpretation and confidence

The strong localhost footprint is COMPATIBLE with internal channel handling or relay/loopback proxying but is NOT a demonstration of it. Port 37777 is documented as a standard service port for several Intelbras video devices; it does not by itself identify Cloud P2P/Relay. The connected devices already existed before recording, so the nonlocal Established endpoints present at first sample cannot be assigned to this single action.

The major unknowns: what the operator did immediately after ENTER, which specific device was interacted with, and whether SIM Next UI showed Cloud(P2P), Cloud(Relay), IP/Domain or failed. No evidence in this metadata can disambiguate. Do not infer vendor service IPs, authenticate a peer, or reconstruct Cloud protocol from tag or service-port alone.

## Safe next observation

Keep SIM Next running but close every visible camera tile. Do NOT delete registered devices. Run from project repository:
    .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Idle
Press ENTER and perform no interaction during the 25-second snapshot. Then (in a separate later run) compare with Live with one camera open. Only capture Wireshark traffic if an authorized live session, after reducing to necessary flows and stripping payload/token/serial before sharing.

The user separately uploaded local-hash-salt.txt with the JSON. Its contents are NOT copied to this report or GitHub; the original pseudonymization salt should be considered exposed. For any NEW shareable series, regenerate the salt locally (loses hash comparability across the old and new series) or otherwise explicitly protect it.

## Release gate

R10 Windows [8/8] build and 6/6 CTest PASS remains the last production code gate. This metadata research does not prove Intelbras Cloud access and changes no C++ software or tests. Do not use SIM Next DLLs, guess app identities, extract vendor secrets, bypass security, execute blocked dh-fwd or merge main. PR #4 draft; F004 Human Review pending.
