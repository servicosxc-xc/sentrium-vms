# F005 RE-001 OBS-02 — Connecting vs Idle, passive SIM Next logs

Observation 27 October 2026 on the operator-owned Windows host, SIMNext.exe version 1.25.4.1203 PID 22076, authentic signature Valid. Source consists of two JSON reports (Connecting 17:55:53 UTC, 25.2 s; Idle 18:15:26 UTC, 26.08 s). This report contains no IPs, endpoint tags, pseudonymization salt, credentials, NVR serial numbers or raw capture data.

## Differential analysis

| Metric | Connecting | Idle |
|---|---:|---:|
| Grouped TCP state records | 322 | 500 |
| Unique LOCAL UDP endpoints | 89 | 112 |
| TCP local wildcard/listen/bound or loopback | 308 | 470 |
| TCP non-local remote-state records | 14 | 30 |
| TCP SynSent records | 5 | 20 |
| Established reciprocal local TCP endpoint pairs | 72 | 91 |
| Loaded modules (names) | 196 | 195 |

**Caveat:** grouped rows are distinct state+port combinations observed during separate ~25 second windows, NOT traffic volume, simultaneous sessions, number of cameras or distinct user-initiated attempts. Both runs recorded the same main process hash and PID. Windows cmdlets are expensive: approx 22 samples per run despite requested sampling 500 ms, so connection timestamps are approximate observations, not actual packet boundaries. The newer Idle snapshot was taken ~20 minutes later, when background retries and leftover sessions may have evolved. Operator confirmation of the UI state remains unreported.

### External TCP port differences (unattributed)

- 37000 TCP in Connecting: 3 SynSent socket records. In Idle: 8 SynSent across three pseudonymized destinations. **No successful 37000 session in either observed window.**
- 37777 TCP in Connecting: 1 established persistent across all samples plus 2 SynSent. In Idle: the same established socket persisted plus 8 SynSent; there was no evidence these related to the particular Cloud action. Port 37777 often represents DVR/NVR service, so is not sufficient evidence of P2P.
- 37002 TCP: absent in Connecting sample; 4 SynSent in Idle. Again no conclusion about protocol or active device.
- 7000, 12246, 37001 and 37719: the exact same TCP socket tuples were Established in BOTH reports, already present at the first sample. These are a defensible long-lived baseline, but cannot be classified without endpoint provenance and device mapping.
- Non-local TCP state rows with remote port 443 differed between observations, consistent with application/web control traffic; not proof of a cloud bootstrap.
- Exactly 5 external TCP tuple+state keys were shared between snapshots: established service ports 7000, 12246, 37001, 37719, 37777.

### UDP and local sockets

- UDP local-port key overlap between runs: 72. Connecting-only: 17. Idle-only: 40. These are LOCAL listening/bound ports, **NOT confirmed UDP peer connections**; no remote UDP address, datagram count, NAT mapping, or encryption/session data present.
- Established loopback TCP rows: 144 vs 182 (72 vs 91 reciprocal pairs). Most of the increase in total TCP records is local/wildcard (308 to 470), not new confirmed off-host streams.
- Idle report included 70 grouped entries with firstSeen at a single sample ~13.415s; 22 UDP ports appeared at that sampling point. Could reflect spontaneous internal socket churn, connection maintenance or snapshot timing; cannot attribute to user action (Idle scenario).
- Module differences were small (two named modules only in Connecting; one only in Idle) and do not demonstrate Cloud implementation.

## What can and cannot be concluded

This **does** show a busy SIM Next background session manager with many persistent local sockets, external TCP services and repeated unresolved TCP connects **even when the operator labeled the capture Idle**. It suggests that simply closing mosaic tiles is not necessarily enough to close device control sessions.

This does **NOT** show packet sizes, UDP peers, Cloud server identities, ownership of individual endpoints by a particular NVR, P2P Direct/Relay outcome or legitimate third-party application bootstrap.

## Next experiment: single controlled live camera

With SIM Next legitimately operating on a user-authorized NVR, leave all registered devices intact. Close all mosaic tiles, then run:
    .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Live
Press ENTER and IMMEDIATELY open exactly one camera from a known remote NVR in EXTRA stream; keep it open during the 25-second sample. Record: whether the feed loaded, exact UI path display Cloud(P2P)/Cloud(Relay)/IP/Dominio (if shown), and approximate second relative to ENTER when opened. Send only the reviewed JSON. The same local hash salt preserves comparisons but was already uploaded once; never redistribute salt or raw captures and consider regenerating it for future new sharing series if needed (which breaks prior tag correlation).

Then compare local ports/established/TCP flow additions across Idle and Live. If nonlocal UDP remains ambiguous as expected, plan a narrowly scoped, passive Wireshark capture on an authorized network, minimizing collection and sharing headers/timing only after redaction.

## Security/release status

No app credential extraction, no MITM/TLS breaking, no endpoint credential guesses, no vendor DLL copying, no bypass of Defender, no dh-fwd blocked binary. Vendor licensing still unresolved. No product source modification or new Windows test gate; R10 6/6 PASS retained. PR #4 draft and main/Human Review untouched.
