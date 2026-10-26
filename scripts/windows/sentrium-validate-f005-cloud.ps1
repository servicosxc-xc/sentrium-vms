<#
.SYNOPSIS
  Validate the current F005 native Cloud-signaling code, once, on Windows.
.DESCRIPTION
  Prevents LNK1168 (sentrium-client.exe still running), does a CMake
  configure/build, then only the Cloud protocol offline checks D-9/D-10.
  This is NOT a live P2P connection test. It neither launches an untrusted
  Cloud binary nor opens a firewall port.
#>
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Push-Location $repoRoot
try {
    $branch = (& git branch --show-current).Trim()
    if ($LASTEXITCODE -ne 0 -or $branch -ne 'foundation/005-device-connectivity') {
        throw "Expected branch foundation/005-device-connectivity; found: $branch"
    }
    $dirty = @(& git status --porcelain)
    if ($LASTEXITCODE -ne 0 -or ($dirty | Where-Object { $_ })) {
        throw 'Working tree is not clean. Review local changes before pulling.'
    }

    $running = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
    if ($running.Count -gt 0) {
        $pids = ($running | ForEach-Object { $_.Id }) -join ', '
        throw "LNK1168 PREVENTED: sentrium-client.exe is running (PID: $pids). Close the app normally, then rerun. This script does not terminate user processes."
    }

    & git pull --ff-only origin foundation/005-device-connectivity
    if ($LASTEXITCODE -ne 0) { throw 'Git fast-forward failed' }

    # A child script shares process environment exports (SENTRIUM_CMAKE etc.)
    # but does not inherit this runner's strict variable/property scope.
    & (Join-Path $PSScriptRoot 'sentrium-preflight.ps1') -VerifyOnly

    # The PRE-FLIGHT enumerates heterogeneous registry entries; it has its
    # own defensive property checks. StrictMode is enabled for the remaining
    # orchestration AFTER the preflight returns.
    Set-StrictMode -Version Latest

    if (-not $env:SENTRIUM_CMAKE -or
        -not (Test-Path -LiteralPath $env:SENTRIUM_CMAKE -PathType Leaf)) {
        throw 'PREFLIGHT did not configure a valid SENTRIUM_CMAKE'
    }

    & $env:SENTRIUM_CMAKE --preset sentrium-debug
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }

    & $env:SENTRIUM_CMAKE --build --preset sentrium-debug
    if ($LASTEXITCODE -ne 0) { throw 'CMake build failed' }

    $ctest = Join-Path (Split-Path $env:SENTRIUM_CMAKE -Parent) 'ctest.exe'
    & $ctest --preset sentrium-debug -R '^D-(9|10)$' --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Native Cloud wire/UDP offline regression failed' }

    $commit = (& git rev-parse HEAD).Trim()
    if ($LASTEXITCODE -ne 0) { throw 'Cannot read Git commit' }

    Write-Host "F005 DEBUG BUILD + D-9/D-10: PASS" -ForegroundColor Green
    Write-Host "Commit: $commit"
    Write-Host 'Cloud bootstrap, Direct/Relay and real P2P login: NOT VERIFIED.'
}
finally {
    Pop-Location
}
