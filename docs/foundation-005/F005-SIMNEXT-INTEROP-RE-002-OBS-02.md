# F005 RE-002 OBS-02 — Full 49-second Wireshark PCAPNG analysis

Date 2026-10-27 (local), capture on 2026-10-28 01:48:40..01:49:29 UTC. Source is an operator-supplied approximately 7.2 MiB PCAPNG file, analyzed offline using only packet metadata. THIS REPORT intentionally omits all source/destination IP addresses, device serials, keys, raw payloads and customer identifiers. Do not commit/distribute the original capture.

## Verified counts

- PCAPNG: Ethernet link type, one network interface, timestamp resolution nanosecond, **15,087 captured frames**, duration **48.957 seconds**. IPv4 UDP 13,687 packets and IPv6 UDP 113; IPv4 TCP 836 and IPv6 TCP 280; remaining link/protocol types small.
- Dominant two-way UDP 5-tuple (shown ONLY as remote UDP port 1639 vs local UDP port 64220): **4,508 packets, 5,484,597 bytes on wire**; receive 4,068 / 5,456,667 bytes, send 440 / 27,930 bytes. Approximately 79.1% of all frame bytes, 81.9% of UDP frame bytes.
- Remote UDP media-like traffic: first received frame >=900 bytes at capture **t=13.913 s**, last at **t=36.482 s**. Traffic before ~13 s and after ~37 s only small control-sized datagrams on this 5-tuple. For interval t=14..36 s (~22 s), traffic averaged **1.92 Mbit/s** (including Ethernet/IP/UDP overhead).
- Dominant packet sizes: 3,265 received Ethernet frames of **1,442 bytes** (UDP payload 1,400 bytes); 390 received frames of 946 bytes (UDP payload 904 bytes); 405 sent 58-byte frames (UDP payload 16 bytes). Total main-flow frames 4,508.
- Incoming type-02 observed-frame IDs (bytes 4..7 of UDP payload, big-endian) form a contiguous sequence of **4,035 unique values with no gaps or duplicates** in this capture. The 405 outgoing short type-03 messages reference one of these received IDs in payload bytes 8..11. Correlation demonstrates an ACK-like relationship, **not proof of a cryptographic authenticated peer**, and measurement at capture host does not represent internet round-trip time.

## Observed UDP framing (empirical, not official protocol specification)

- Across all UDP packets, first four payload bytes occurred with these counts: hexadecimal 12 01 10 00: **1,725**; 12 01 10 01: **1,710**; 12 01 10 02: **6,481**; 12 01 10 03: **2,641**. Other message types are also present. The meanings are NOT formally established; types 00/01 behave like short exchanges, 02 carries data, 03 appears ACK-like.
- For **all 6,481** observed type-02 UDP datagrams: the two-byte big-endian unsigned field at offsets 8..9 equals exactly UDP_PAYLOAD_LENGTH minus **16**. This is consistent with a 16-byte envelope. No bytes beyond the fixed observable header are reproduced here.
- The main-flow packet type is 12 01 10 02 for 4,035 received packets; 405 corresponding short outgoing type-03 messages. The observed sequence value increments by one, with zero gaps/duplicates in this capture.
- The production-facing C++ experimental parser at libs/sentrium-device/cloud_ptcp.cpp currently expects ASCII magic PTCP and a 24-byte header. **Zero captured UDP payloads contained the ASCII token PTCP.** Therefore the existing PTCP parser will NOT parse these UDP envelopes directly. DO NOT conclude that no separate inner PTCP layer exists, nor replace the current implementation blindly.
- The capture confirms a real repeatable UDP envelope and message acknowledgments; does NOT reveal initial account/app authentication, legitimate vendor Cloud bootstrap, rendezvous, NAT negotiation, Direct vs Relay mode, or whether the remote endpoint is the DVR or a forwarding server.

## Timeline and what remains ambiguous

| Relative time | Actual observation |
|---|---|
| 0–12 s | Dominant 5-tuple has sparse small datagrams |
| ~13.9 s | UDP 900+ byte datagrams start |
| 14–35 s | Sustained approximately 1.9 Mbit/s media-like inbound traffic |
| ~36.48 s | Last 900+ byte received datagram |
| 37–49 s | Main 5-tuple returns to occasional control-sized packets |

This aligns with a camera opened and closed during the intended test, but the operator has not yet explicitly confirmed whether video displayed normally and what the SIM Next UI indicated: Cloud P2P, Cloud Relay, IP/Domain or unspecified. That user observation is required for final route attribution.

## Next milestone: authorized real Cloud session bootstrap

1. Obtain operator's confirmation that exactly one remote camera from an owned/authorized recorder played between t~14 and t~36.5 and the route displayed in SIM Next, if any.
2. Correlate the remote endpoint (only locally, do NOT commit its IP) to the actual recorder's public route, when known. A stream to a public IP does NOT alone distinguish Direct from Relay.
3. Investigate **initial app/device connection before existing sessions** using controlled clean application start and passive metadata/packet capture, with an authorized device and no extraction of vendor app keys, passwords, TLS session secrets or proprietary identities.
4. If a legitimate third-party SDK/app authorization is needed, obtain it officially; do not claim production Cloud interoperability solely from matching UDP envelope layout.

## Security/engineering gates

No PCAPNG, raw payload, sensitive IP, endpoint hash, local salt, serial, credential or SIM Next binary was committed. No C++ library/CTest modification here, and no Windows build needed. R10 Windows [8/8] build, D-41..D-46 6/6 PASS still valid as **LOCAL SYNTHETIC lab**. PR #4 draft, main and F004 Human Review untouched. No dh-fwd Defender-blocked or vendor bypass.
## Operator confirmation received — P2P-labelled SIM Next connection, one camera open/close

Operator confirmed after RE-002 PCAP: capture began with no camera open, then opened one channel, then closed it; the device connection in SIM Next UI was labelled **P2P**. This is direct operator observation, not data decoded from the PCAP. The actual rendered picture quality was not separately confirmed.

This resolves the previous UI/sequence uncertainty: in the authorized P2P-labelled user session, UDP remote port 1639 ↔ local 64220 produced a high-volume inbound stream from approximately capture t=13.913s through t=36.482s, strongly correlated with the opening and closing sequence. Media-bearing UDP transport during a **P2P-labelled SIM Next session** is now supported by both network evidence and operator observation.

CRITICAL LIMIT: P2P in the SIM Next UI does not prove UDP endpoint is the recorder itself or that no relay/intermediate vendor infrastructure is used. Does not show vendor app identity, bootstrap/authorization, NetSDK tunnel or third-party app permission. The observed 12 01 10 xx UDP envelope with 16-byte header remains distinct from the experimental C++ PTCP/24-byte ASCII PTCP parser on current evidence.

### Next RE-003 research gate (planned; NOT RUN)

Study the initial authorized SIM Next startup, not additional established video flows. Leave authorized recorder registrations untouched. Close SIM Next normally; start a local Wireshark capture on the actual network interface for ~5–10 seconds with the app absent; launch SIM Next and observe one test recorder coming online; open just one camera extra stream. Record relative times of app launch, device online, camera open, UI route P2P, and stop capture after a brief active period. A few other registered recorders may automatically connect; do not attribute destinations without independent evidence. This is passive observation, not removal/reset of devices/accounts or modification of security. Keep raw PCAP local; share redacted flow/timing statistics only. No copying/decoding vendor client credentials, SIM Next binaries, decryption keys, authentication bypass or Defender-blocked software.

No C++/CTest modifications here; R10 Windows 6/6 PASS remains. PR #4 draft, main and F004 Human Review untouched.