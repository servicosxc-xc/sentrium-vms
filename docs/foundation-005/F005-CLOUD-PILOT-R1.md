# SENTRIUM F005 — Cloud pilot R1: SECURITY HOLD

**Status:** SUSPENDED — NOT SAFE TO EXECUTE ON USER WORKSTATION  
**Supersedes:** previous field-trial instructions for third-party dh-fwd (historical instructions remain available through Git revision history, not as approved runbook).

On 26 October 2026, Microsoft Defender quarantined the published Windows x64 `dh-fwd` v0.2 binary, identifying it as `Trojan:Win32/Bearfoos.A!ml`. Execution was prevented. The SHA-256 matched the upstream release; this does not establish that it is benign.

**Do not execute, restore from quarantine, add Defender exclusions, rename or repackage the file, or build the flagged tool from source solely to evade detection.** No new alternate executable is recommended at this time.

See the complete evidence and decisions in:
`docs/foundation-005/F005-CLOUD-SECURITY-INCIDENT-001.md`.

## Development status

- LAN / NetSDK: functional, existing baseline.
- AutoRegistration: the operator demonstrated one NVR Connected with 8 real channels.
- Cloud: C++ `CloudDeviceSession` and endpoint protocol-neutral contracts are available; the independent Cloud bootstrap/network provider remains **BLOCKED** by both technical and security requirements.
- The experimental UI for NetSDK login via external `127.0.0.1` port is **disabled pending a reviewed, trusted provider**.
- The associated PowerShell pilot fails closed before requesting IDs/credentials or launching any binary.

## Next security-approved engineering work

Perform **read-only source review** and develop a self-contained and auditable Cloud provider design with no SIM Next runtime files. Any new runtime proof requires source provenance, signed/attested binaries where available, authorization to use Cloud infrastructure, a completed security assessment and an isolated test environment. Do not assume Open Source license permits use of third-party Cloud authentication constants.

Any decision to resume a pilot must be explicitly reviewed and recorded. Do not infer clearance from a matching release hash or a single anti-virus result.

**No Cloud connection through this pilot has been established.**
