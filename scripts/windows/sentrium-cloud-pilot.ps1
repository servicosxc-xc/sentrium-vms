# SENTRIUM F005 — ONE real-device Cloud/P2P pilot using an optional,
# independently built, MIT-licensed bridge. No SIM Next files are used.
# Does NOT edit firewall/NAT, install or download dependencies, or persist
# Cloud identifiers/credentials. Bridge lifecycle is owned by this terminal.
[CmdletBinding()]
param(
    [string]$BridgeExe = "C:\Temp\SentriumCloudPilot\sentrium-cloud-pilot.exe",
    [Parameter(Mandatory=$true)][ValidateRange(1,65535)][int]$RemotePort,
    [ValidateRange(1024,65535)][int]$LocalPort = 47377,
    [ValidateSet("smartpss","dmss")][string]$Profile = "smartpss",
    [switch]$CloudAuth,
    [switch]$Relay,
    [switch]$AppRelay,
    [switch]$ProbeOnly
)
$ErrorActionPreference = "Stop"

# SAFETY HOLD (2026-10-26): Microsoft Defender quarantined the upstream
# dh-fwd Windows v0.2 binary (Trojan:Win32/Bearfoos.A!ml). No exception,
# restoration, Defender exclusion, alternate filename or source rebuild is an
# acceptable workaround without separate independent security clearance.
# The Cloud pilot must fail closed BEFORE probing ports or requesting IDs,
# credentials or launching ANY executable, regardless of supplied -BridgeExe.
throw "F005 Cloud pilot SUSPENDED: Defender detected Trojan:Win32/Bearfoos.A!ml in the third-party bridge. Do not run/rebuild/bypass. See docs/foundation-005/F005-CLOUD-SECURITY-INCIDENT-001.md."

if (-not (Test-Path -LiteralPath $BridgeExe -PathType Leaf)) {
    throw "Bridge executable not found. Build the local independent Cloud pilot as documented in docs/foundation-005/F005-CLOUD-PILOT-R1.md"
}
if ($RemotePort -eq $LocalPort) {
    Write-Host "Notice: the remote service port and local port have the same number, but are separate endpoints." -ForegroundColor Yellow
}

# Never send a Cloud ID, NVR user, NVR password or full bridge logs back to chat.
$cloudId = Read-Host "Cloud identifier of your own recorder (do not send it here)"
if ([string]::IsNullOrWhiteSpace($cloudId)) {
    throw "Cloud identifier is required"
}
if ($cloudId.Length -gt 128 -or $cloudId -match '\s') {
    throw "Cloud identifier has unexpected length or whitespace"
}

if (-not $ProbeOnly) {
    # Only test binding the chosen local TCP port. This does not imply
    # any real tunnel, route or recorder connectivity.
    $tcp = [System.Net.Sockets.TcpListener]::new(
        [System.Net.IPAddress]::Loopback, $LocalPort)
    try {
        $tcp.Start()
    } catch {
        throw "Local TCP port $LocalPort is already in use. Choose another unused port."
    } finally {
        try { $tcp.Stop() } catch {}
    }
}

$authUser = $null
$authPassword = $null
try {
    $env:SENTRIUM_PILOT_CLOUD_ID = $cloudId
    if ($CloudAuth) {
        $authUser = Read-Host "Cloud authentication username (temporary test account preferred)"
        $secure = Read-Host "Cloud authentication password" -AsSecureString
        $authPassword = [System.Net.NetworkCredential]::new("", $secure).Password
        Remove-Variable secure
        if (-not $authUser -or -not $authPassword) {
            throw "Both username and password are required in CloudAuth mode."
        }
        $env:SENTRIUM_PILOT_USER = $authUser
        $env:SENTRIUM_PILOT_PASSWORD = $authPassword
    } else {
        Remove-Item Env:\SENTRIUM_PILOT_USER -ErrorAction SilentlyContinue
        Remove-Item Env:\SENTRIUM_PILOT_PASSWORD -ErrorAction SilentlyContinue
    }
    $argsList = @("--app", $Profile, "--port", "$($LocalPort):$($RemotePort)")
    if ($Relay) { $argsList += "--relay" }
    if ($AppRelay) { $argsList += "--app-relay" }
    if ($ProbeOnly) { $argsList += "--test" }

    $hash = (Get-FileHash -LiteralPath $BridgeExe -Algorithm SHA256).Hash
    Write-Host "SENTRIUM standalone Cloud pilot (bridge SHA256 prefix $($hash.Substring(0,12)))"
    Write-Host "Cloud ID: <redacted> | profile: $Profile | remote service: $RemotePort | local TCP: $LocalPort"
    Write-Host "Credentials are not supplied in OS command line, but remain in the child process environment/memory."
    if ($ProbeOnly) {
        Write-Host "Mode: upstream device-port probe; NOT a P2P tunnel."
    } else {
        Write-Host "If tunnel reports success, open SENTRIUM > Dispositivos > Novo > Cloud / P2P"
        Write-Host "Enter local port $LocalPort and the recorder's ordinary NetSDK username/password."
        Write-Host "Keep this terminal open while SENTRIUM is using the tunnel; Ctrl+C to stop."
    }
    Write-Host "IMPORTANT: upstream may print Cloud ID/addresses. Do not paste raw output or logs into chat."
    & $BridgeExe @argsList
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) {
        Write-Warning "Bridge exited with code $exitCode (no Cloud connection has been proven)."
    }
} finally {
    Remove-Item Env:\SENTRIUM_PILOT_CLOUD_ID -ErrorAction SilentlyContinue
    Remove-Item Env:\SENTRIUM_PILOT_USER -ErrorAction SilentlyContinue
    Remove-Item Env:\SENTRIUM_PILOT_PASSWORD -ErrorAction SilentlyContinue
    $authPassword = $null
    $authUser = $null
    $cloudId = $null
}
