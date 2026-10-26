# SENTRIUM F005 — Cloud P2P standalone pilot R1

**Date:** 2026-10-26  
**Status:** PILOT SOURCE IMPLEMENTED / NO REMOTE CLOUD TEST YET / NOT PRODUCTION  
**Prior result:** owner screenshot confirms NetSDK AutoRegistration device `Connected`, eight channels, with registration listener active. This proves one AutoReg device path, not Cloud functionality.

## Goal

Reach an owned Intelbras/Dahua remote recorder through an independently implemented P2P tunnel and feed its **actual TCP NetSDK port** to SENTRIUM as `127.0.0.1:localPort`. No SIM Next installation, DLL, configuration or private bootstrap material is required by this pilot. No fabricated Direct/Relay state.

**This is an independent helper-based proof, NOT native C++ bootstrap/managed P2P.** If compatible, port/adapt only the necessary transport code with full rights and operator-approved integration later.

## Protocol proof source: dh-fwd (MIT)

- Repository: https://github.com/undervolter/dh-fwd
- Source revision reviewed: `c26ce4f4b7051bfd0b8d07d8a22400f52cb5142c`.
- MIT license: upstream `LICENSE`, copyright (c) 2026 undervolter. Do not vendor, modify or redistribute without preserving the upstream copyright/LICENSE and validating applicability to infrastructure Cloud terms.
- Upstream has Go 1.25 module and explicitly supports local forwarding of arbitrary device TCP ports, e.g. `-p 47377:37777`, `--app smartpss|dmss`, `--relay`, `--app-relay`, `-t 1` with user/password. It discovers the real service ports when device Info metadata allows; do not hardcode `37000` or `37777` as universal.
- Notes: Intelbras-specific Cloud compatibility is **unproven** in the user's NVRs. Unknown whether direct P2P, relay, authorization or device authentication will succeed. Old firmware may use no Cloud channel auth; recent devices may require type 1 auth.
- Another repository `blackjid/dahua-p2p` uses direct-only RTSP tunneling; this pilot selects the MIT `dh-fwd` implementation because it documents arbitrary TCP forwarding plus Relay.
- The README and current code in the upstream repository may change; the pinned SHA avoids silent source drift.

## Building the independent pilot on Windows (manual, opt-in)

**No dependency is installed or downloaded by SENTRIUM itself.** To use this experimental pilot, the operator chooses to compile a reviewed Go helper in a local temporary directory, not in the SENTRIUM production tree. You need Go 1.25+ on the machine (first run `go version`).

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity

# Run these only if Git and Go >= 1.25 are already available:
New-Item -ItemType Directory "C:\Temp\SentriumCloudPilot" -Force | Out-Null
git clone https://github.com/undervolter/dh-fwd.git "C:\Temp\SentriumCloudPilot\dh-fwd"
cd "C:\Temp\SentriumCloudPilot\dh-fwd"
git checkout --detach c26ce4f4b7051bfd0b8d07d8a22400f52cb5142c

# Go resolves MIT upstream module through the pinned working tree, while
# building our own minimal credential-safe launcher. Nothing is copied to Git.
go build -o "C:\Temp\SentriumCloudPilot\sentrium-cloud-pilot.exe" "C:\Projetos\SENTRIUM VMS\sentrium-vms\scripts\windows\sentrium-cloud-pilot-main.go"
```

The test runner accepts the Cloud ID and optionally Cloud-specific device user/password using environment variables scoped to the launch, avoiding passwords in **OS process command-line arguments**. Environment/process-memory exposure to sufficiently privileged local software still applies; use a limited temporary test account and do not paste full output logs.

### One end-to-end Cloud attempt

With that helper built, from another Developer PowerShell:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"

# Replace 37777 with the remote recorder's confirmed NetSDK TCP service
# port when different; the 37000 from historical tests is NOT a universal value.
.\scripts\windows\sentrium-cloud-pilot.ps1 -RemotePort 37777 -LocalPort 47377
```

The command prompts for the device's **Cloud ID**, without storing it. It defaults to the SmartPSS cloud profile and unauthenticated channel negotiation (type 0), which may be unavailable for devices with newer firmware. For a DMSS-bound recorder add `-Profile dmss`; for recent devices requiring **authorized Cloud channel** authentication add `-CloudAuth` (prompts privately for cloud-user/pass rather than passing credentials at OS launch). `-Relay` may be used to test Relay explicitly, not to assert that it occurred.

If the pilot establishes a live TCP forwarder on `127.0.0.1:47377`, keep its console open. **Only then**, in the SENTRIUM UI:

1. `Dispositivos > Novo > Cloud / P2P` (experimental external local bridge).
2. Enter the remote NVR name; the **local port 47377**, not the remote 37777/37000.
3. Enter the recorder's usual **NetSDK** user/password, which may differ from Cloud-channel credentials.
4. Connect. Confirm `Connected` and an actual device channel count.

This works without SIM Next, but still depends on an explicitly launched, external open-source prototype. The bridge process is not yet owned/managed by the SENTRIUM client and the underlying Cloud route remains `Unknown`, even when NetSDK succeeds. If bridge exits or TCP listener is absent, the client's NetSDK attempt must fail, not pretend to connect.

### If the first attempt fails

Use the error category, not endless repeated benchmarking:

- **Go not available / compile error:** report the relevant error only. Do not install new dependencies automatically.
- **404** in Cloud lookup: verify SmartPSS/DMSS profile and authorized Cloud ID. Do not publish the identifier.
- **403 / authentication required:** run `-CloudAuth` with an authorized temporary account; never share passwords.
- **Bridge logs listening, but NetSDK login fails:** check remote service port, source's Info-probed port mapping, and login user/password. Some protocol combinations may support RTSP and not NetSDK DVRIP.
- **Successfully connected:** record real number of channels, disconnection/reconnection behavior and whether link used Direct or Relay only if the bridge explicitly reports it.

**No P2P Cloud or NetSDK success has yet been observed via this new pilot in this branch.**

## Boundaries

- Not production or a shipping cloud dependency; no public listener, port forwarding, SDK download, firewall changes, or credentials hardcoded in repository.
- OSS MIT terms concern software copyright and **do not confer rights to any cloud-service accounts, access tokens, servers or protocol use**. Test only devices that the operator owns or is authorized to administer.
- Never use SIM Next binaries, login secrets or extracted bootstrap credentials.
- No claims of validated Direct/Relay classification, independent full feature compatibility, stability, clean Windows support or 16/32 camera scalability before field evidence.
