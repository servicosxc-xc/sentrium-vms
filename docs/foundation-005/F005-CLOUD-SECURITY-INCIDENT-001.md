# F005 — Security hold: third-party Cloud/P2P pilot

**Date/time:** 2026-10-26 09:47 (user's Windows Defender screenshot, Brazil timezone)  
**Status:** **SECURITY HOLD — DO NOT EXECUTE**  
**Scope:** third-party Windows binary `dh-fwd_win_x64.exe` from `undervolter/dh-fwd` release `v0.2`.

## Observed evidence

- Windows Defender identified `Trojan:Win32/Bearfoos.A!ml` and reported **Quarantined**.
- Download location: `C:\Temp\SentriumCloudPilot\dh-fwd_win_x64.exe`.
- SHA-256 calculated by the operator matched the hash published by the repository release: `0bf756591bc9e168e5d0e95dc21fe1a807a3b3afcc1ace1959b3cee0ed92d93a`. **Hash equality is provenance only, not a clean-bill-of-health.**
- PowerShell rejected execution: `NativeCommandFailed` / "arquivo contém um vírus ou software possivelmente indesejado". No successful start of the binary or Cloud tunnel is evidenced.
- No evidence confirms a false positive; do not assert malware infection either. The alert was on the quarantined file.

## Read-only source review (not equivalent to malware analysis)

Pinned upstream source revision used for earlier evaluation: `c26ce4f4b7051bfd0b8d07d8a22400f52cb5142c`, hosted at `https://github.com/undervolter/dh-fwd`.

- `core/updater.go` has an update-check and `selfUpdate` routine which may download a release asset and replace the executable, with an operator prompt. This is a code-level risk requiring review, **not proof of malicious intent**.
- `core/helpers.go` declares Cloud bootstrap server settings and stock client authentication constants. These must **not be copied or embedded into SENTRIUM** absent explicit legitimate integration authority and an independently verified source.
- `core/cli.go` supports Cloud-channel user/password. `core/logger.go` can create local log files. These are audit surfaces requiring credential redaction verification.
- These observations do not establish that the published executable corresponds reproducibly to the reviewed source, or that the binary is safe.

## Mandatory actions taken

1. `scripts/windows/sentrium-cloud-pilot.ps1` **fails closed immediately**, before fetching IDs, passwords, binding ports or executing any binary. No opt-out switch.
2. `Main.qml` Cloud pilot is explicitly disabled and does not permit submitting NetSDK login credentials into an unaudited local TCP bridge.
3. Existing functional LAN and **8-channel AutoRegistration** paths remain untouched.
4. Do not disable Defender, add exclusions, unquarantine, rename, repack, re-download an alternate binary or compile the same flagged tool *for the purpose of bypassing detection*.

## Separate future Cloud development

SENTRIUM still targets standalone Cloud (no SIM Next runtime dependency), but a provider may proceed only after:

- Independent source and binary assessment / Microsoft Security Intelligence classification; review the published release/provenance, all network endpoints, update mechanism and credential handling.
- Valid integration rights for the Cloud service, credentials and redistributable software (MIT code license alone does not grant account/Cloud usage rights).
- A reviewed prototype in an isolated, permissioned lab after security clearance. No prototype execution on the user's primary workstation during the hold.
- Verification of device ownership/authorization, tunnel handshake, NETSDK login, channel list, teardown and route provenance.
- Mark `CloudRoute` Unknown until Direct/Relay is actually measured, and do not claim Cloud PASS on the presence of a local port.

The `CloudDeviceSession` C++ orchestration remains useful independent of the quarantined program.

## Optional outside review

Microsoft's submission portal: https://www.microsoft.com/en-us/wdsi/filesubmission . For the quarantined release, request that **the publisher** submits the sample or coordinate a safe submission; **do not restore or execute it** just to upload a sample. The result may confirm or reject a false positive.

**Governance:** F005 IN IMPLEMENTATION; Cloud standalone provider BLOCKED (security and bootstrap); F004 pending; no merge to `main`, no Human Review approval.
