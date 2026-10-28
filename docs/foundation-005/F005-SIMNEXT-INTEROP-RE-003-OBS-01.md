# F005 RE-003 OBS-01 — Fresh SIM Next startup, direct-to-site UDP P2P supported

28 October 2026 UTC. Operator recorded a real Wireshark session with Wireshark started before SIM Next, then SIM Next launched, P2P shown in UI, one channel visualized and closed before capture ended. PCAPNG: 5,135 captured frames, 73.320 seconds, Ethernet. Input analyzed offline read-only; **original PCAP, payload, device serial, WAN/LAN address, tokens and credentials remain outside GitHub**.

## Decisive operator WAN-IP correlation

The operator supplied the **current WAN IPv4 address of the network containing the authorized DVR**. Offline analysis compared that exact address, without copying its literal value into this repository, against BOTH RE-002 (previous 48.957-second video open/close capture) and RE-003 (fresh startup):

- RE-002: 4,508 UDP frames / 5,484,597 wire bytes between that WAN IPv4 at UDP remote port 1639 and operator local UDP 64220, with inbound media-volume rise during the video tile's opening and cessation after close.
- RE-003: 3,965 UDP frames contacting the SAME WAN IPv4, including an initial UDP probe to remote port 1132 and 3,964 frames / 3,586,029 wire bytes on remote port 1131 ↔ local port 51699. The main flow consists of 3,086 inbound frames and 878 outbound; 3,489,905 inbound wire bytes and 96,124 outbound wire bytes.
- The SIM Next UI showed P2P; remote WAN endpoint matched the real **recorder site's WAN IP in BOTH captures**, while remote UDP port changed between captures. **Strong evidence of DIRECT-TO-SITE P2P as the media transport path**, not a vendor relay carrying the observed large media flow. This does NOT prove an IP packet reached the DVR directly rather than a NAT/router/forwarder at that site, nor that separate cloud discovery/signaling servers were absent.

## RE-003 timeline and visible wire shapes

| Capture time | Observable event |
|---|---|
| ~11.20 s | DNS lookups for Intelbras-related hostnames, some likely SIM Next infrastructure; the capture itself cannot attribute every DNS lookup to a particular process. |
| ~11–14 s | HTTPS connections and further DNS lookups; do not classify every connection as login/authentication. |
| 40.767 s | First UDP contact with operator-confirmed remote WAN IP at remote port 1132 (single observed probe). |
| 41.369 s | UDP exchange starts against SAME remote WAN IPv4 at remote port 1131; small bidirectional packets; observed 12 20 D0 07, D1 07, D2 07 exchange before 12 01 10 xx payload/ack family. |
| 42.07–42.16 s | Multiple 12 01 10 xx control/data messages already exchanged; some >=900 B packets even before high-bandwidth streaming, so DO NOT equate first large packet with the start of video. |
| ~56 s | Sustained ~220–230 KB/s UDP inbound bulk video-like traffic begins. |
| ~70 s | High-volume media ceases after channel closed; sporadic control packets continue to ~72.9 s. |

Observed DNS query names in the startup phase included intelbrasp2p.com.br (~11.21s), api-cronos.intelbras.com.br (~11.20s), api.simnext.isec.intelbras.com.br (~12.59s), cronos-simnext.intelbras.com.br (~14.07s), and logs.isec.intelbras.com.br (~14.37s). These **public hostnames** are contextual evidence of manufacturer services in use; no IPs/response payloads or assumptions about which server authorizes P2P are committed. No request URLs, path, tokens or session secrets are analyzed.

Within the remote-site 1131 main flow, small messages used headers beginning 12 20 D0 07 / D1 07 / D2 07 during initial negotiation, then 12 01 10 xx (00/01/02/03 plus 05/06). The prefixes are an empirical message classification, **not a complete protocol specification or proven authentication sequence**. In prior RE-002 the bulk data had a 16-byte 12 01 10 02 envelope; the existing SENTRIUM experimental PTCP parser expects ASCII PTCP 24-byte framing and cannot be treated as interchangeable.

## Engineering impact

RE-003 meets the route-attribution gate: **real remote site identified, and high-volume UDP packets exchanged directly with that site's public WAN IP while SIM Next reported P2P**. Main remaining blocker is the authorized discovery/authenticated rendezvous/peer validation and third-party client identity; packet header discovery alone is not sufficient to log an independent VMS into Intelbras Cloud.

Next useful milestone is a controlled passive comparison of startup behavior on a legitimately permissioned site; classify early manufacturer DNS/443 signaling versus endpoint NAT traversal and subsequent channel media. Do not copy identity/keys/certificates from SIM Next, guess proprietary service passwords, intercept others' sessions, disable Defender, redistribute vendor DLLs, or build a provider that simply emulates unlicensed app credentials. Obtain official app license/credentials when required.

Capture is a sensitive site-specific network artifact. No raw PCAP has been added to the repository; keep it local and share only sanitized summaries. No modifications to C++/CTest or runner, and no build needed. R10 Windows local synthetic D-41..D-46 6/6 PASS stands. PR #4 draft, main/F004 Human Review intact.