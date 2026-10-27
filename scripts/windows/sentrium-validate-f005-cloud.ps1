<#
.SYNOPSIS
  Validate Foundation-005 R8 verified datagram stream (focused build).
.DESCRIPTION
  One command after updating local checkout: check branch and worktree,
  fast-forward, PRE-FLIGHT, configure/build Debug and run D-27 to D-34.
  D-19 through D-26 passed on the operator's R7 Windows checkpoint.
  Other earlier component tests are not repeated.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Push-Location $repoRoot

try {
    $expectedBranch = 'foundation/005-device-connectivity'
    $branch = (& git branch --show-current).Trim()
    if ($LASTEXITCODE -ne 0 -or $branch -ne $expectedBranch) {
        throw "Expected branch $expectedBranch; found: $branch"
    }

    $changes = @(& git status --porcelain)
    if ($LASTEXITCODE -ne 0 -or @($changes | Where-Object { $_ }).Count -ne 0) {
        throw 'Working tree is not clean. Review local changes before proceeding.'
    }

    $running = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
    if ($running.Count -gt 0) {
        $pids = ($running | ForEach-Object { $_.Id }) -join ', '
        throw "LNK1168 PREVENTED: sentrium-client.exe running (PID $pids). Close it normally and rerun. No process was terminated."
    }

    & git pull --ff-only origin $expectedBranch
    if ($LASTEXITCODE -ne 0) {
        throw 'Git fast-forward failed.'
    }

    # Pinned roots for fresh Developer PowerShell sessions; exact PRE-FLIGHT
    # still validates each SDK binary. Never overwrite explicitly set paths.
    $knownRoots = [ordered]@{
        'SENTRIUM_QT_ROOT' = 'C:\Qt\6.12.0\msvc2022_64'
        'SENTRIUM_INTELBRAS_SDK_ROOT' = 'C:\Temp\IntelbrasSDK\NetSDK64'
        'SENTRIUM_PLAYSDK_ROOT' = 'C:\Temp\IntelbrasSDK\PlaySDK64'
    }
    foreach ($setting in $knownRoots.GetEnumerator()) {
        if ([string]::IsNullOrWhiteSpace(
                [Environment]::GetEnvironmentVariable($setting.Key,'Process')) -and
            (Test-Path -LiteralPath $setting.Value -PathType Container)) {
            [Environment]::SetEnvironmentVariable(
                $setting.Key, $setting.Value, 'Process')
        }
    }

    # PRE-FLIGHT registry checks run before StrictMode Latest.
    & (Join-Path $PSScriptRoot 'sentrium-preflight.ps1') -VerifyOnly
    Set-StrictMode -Version Latest

    if (-not $env:SENTRIUM_CMAKE -or
        -not (Test-Path -LiteralPath $env:SENTRIUM_CMAKE -PathType Leaf)) {
        throw 'PRE-FLIGHT did not set a valid SENTRIUM_CMAKE executable.'
    }

    & $env:SENTRIUM_CMAKE --preset sentrium-debug
    if ($LASTEXITCODE -ne 0) {
        throw 'CMake configure failed.'
    }

    $logDir = Join-Path $repoRoot 'out\sentrium-debug'
    if (-not (Test-Path -LiteralPath $logDir -PathType Container)) {
        throw "Build directory was not created: $logDir"
    }

    $buildLog = Join-Path $logDir 'f005-cloud-build-latest.log'
    # Build the device library and Cloud tests, not unrelated Qt/media/UI.
    & $env:SENTRIUM_CMAKE --build --preset sentrium-debug --target sentrium-device-tests > $buildLog 2>&1
    $buildCode = $LASTEXITCODE
    if ($buildCode -ne 0) {
        Get-Content -LiteralPath $buildLog -Tail 65
        throw "CMake build failed (exit $buildCode). Full evidence: $buildLog"
    }
    Get-Content -LiteralPath $buildLog -Tail 4

    $ctest = Join-Path (Split-Path $env:SENTRIUM_CMAKE -Parent) 'ctest.exe'
    if (-not (Test-Path -LiteralPath $ctest -PathType Leaf)) {
        throw "CTest executable not found: $ctest"
    }

    & $ctest --preset sentrium-debug -R '^D-(27|28|29|30|31|32|33|34)$' --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        throw 'D-27 to D-34 verified datagram stream tests failed.'
    }

    $commit = (& git rev-parse HEAD).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw 'Cannot read Git commit.'
    }

    Write-Host 'F005 DEVICE-TESTS DEBUG BUILD + D-27 to D-34: PASS' -ForegroundColor Green
    Write-Host "Commit: $commit"
    Write-Host 'Cloud bootstrap, Direct/Relay and real P2P login: NOT VERIFIED.'
}
finally {
    Pop-Location
}
