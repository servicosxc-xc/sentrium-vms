# F005 RE-001 OBS-03 — Live versus Idle versus Connecting

Observation: 27 October 2026; actual operator SIM Next Live JSON generated 18:43:49 UTC, duration 25.99 s. Same SIMNext.exe 1.25.4.1203 PID 22076 as two previous captures; signature Valid. No raw JSON, IPs, endpoint hashes, raw packets or pseudonymization salt committed.

## Measured grouped socket state records (NOT concurrent sessions, packet counts or throughput)

| Metric | Connecting | Idle | Live |
|---|---:|---:|---:|
| TCP | 322 | 500 | 446 |
| UDP LOCAL ports | 89 | 112 | 104 |
| TCP local/wildcard | 308 | 470 | 426 |
| TCP off-host destination | 14 | 30 | 20 |
| TCP SynSent | 5 | 20 | 11 |
| Reciprocal loopback TCP Established pairs | 72 | 91 | 91 |

Important: 535 out of 550 Live rows were already present at first sampled instant (t=0). All 104 UDP local source ports were present at the first sample. Approx 22 system snapshot cycles took place during 25.99 s, despite 500ms configured sleep, due to Windows cmdlet overhead.

Idle-to-Live exact tuple/state intersection 459 rows, Idle-only 153 (129 TCP /24 UDP), Live-only 91 (75 TCP /16 UDP). Eighty-eight UDP source ports were present in both; 24 only Idle, 16 only Live. This indicates socket churn; UDP peer addresses, bytes, packet timing or session state are NOT collected.

All nine off-host TCP Established tuples in Live were also established in Idle, and were present at Live t=0: remote service ports 7000, 12246, 15832, 28295, 37001, 37719, 37777, 44431 and 57314. One 443 TCP session in Idle was absent from Live. No new off-host Established TCP was observed after Live first sample. A newly established UDP media flow could still exist and is NOT ruled out.

TCP remote 37000/37002/37777 entered SynSent repeatedly in Idle AND Live without reaching Established during either snapshot: Idle (8/4/8 respectively); Live (5/2/4 respectively). These are background/retry candidates; do not classify as Cloud bootstrap, P2P or Relay based on port.

Exact action timing and SIM Next UI mode (Cloud P2P, Cloud Relay, IP/Domain, failure or no UI classification) were NOT supplied. Operator has not yet confirmed whether actual video appeared during Live. Thus the local control/session behavior cannot be assigned to a specific camera or authorized NVR.

## Engineering conclusion

Snapshot-based socket enumeration exhausted its useful signal for this experiment. Live sessions may preexist the capture, and the network media path may be UDP. Stop treating socket-port counts as progress toward a Cloud handshake and switch to one targeted passive packet capture.

## RE-002 follow-up — passive Wireshark capture, NOT RUN

1. Leave existing SIM Next devices registered. Close all mosaic tiles, select ONE owned/authorized remote NVR/camera; do not interfere with other customer connections.
2. Start Wireshark packet capture on the active NIC with NO open camera. Capture ~10 s baseline.
3. At ~T+10 s open exactly one camera on EXTRA stream, leaving it visible ~20 s. Record if the image actually opened and what UI route SIM Next reports (Cloud P2P, Cloud Relay, IP/Dominio or not shown). Do not infer route from TCP ports.
4. Close just that camera at ~T+30 s. Continue 10 s and stop at ~T+40 s.
5. Save any raw PCAPNG ONLY locally under ignored out/simnext-interop, never into the project GitHub nor chat. Raw captures can contain IP, serial, session tokens, credentials and unrelated devices.
6. Inspect Wireshark Statistics -> Conversations -> UDP and TCP, sort by Bytes. Focus on flow packet counts, byte changes and temporal correlation with T+10/T+30. With the display filter udp || tcp, recognize it is NOT a capture filter. Keep display filtering flexible rather than guessing port 37777 or 37000.
7. Only share redacted endpoint and traffic metadata or screenshots with addresses/serials redacted; no raw PCAP or payloads. If more precise analysis needed, prepare a sanitized exported metadata-only summary with tshark after confirming it is installed, no keys or headers containing tokens.

Official references: https://wiki.wireshark.org/CaptureFilters ; https://wiki.wireshark.org/Displayfilters .

## Status

R10 Windows Debug build and D-41..D-46 6/6 PASS remains last code gate. No source/tests changed, no new Windows build. PR #4 draft; main and F004 Human Review intact. No independent Intelbras Cloud authentication established; no vendor app secrets, DLL redistribution, TLS bypass or blocked dh-fwd.