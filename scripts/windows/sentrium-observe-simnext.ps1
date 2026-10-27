<#
.SYNOPSIS
  Passive metadata capture for authorized SIM Next interoperability research.
.DESCRIPTION
  Read-only observation of local process names, loaded module NAMES, signature
  status, and Windows TCP/UDP connection metadata. Does not attach a debugger,
  read process memory, export credentials or capture packet payloads.
  Remote IPs are salted/pseudonymized before writing reports. The salt is kept
  only in out/simnext-interop (already excluded from Git).
.EXAMPLE
  .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Connecting
.EXAMPLE
  .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Live -ProcessIds 1234,5678
#>
[CmdletBinding()]
param(
    [ValidateSet('Idle','Connecting','Live','Relay','Disconnecting')]
    [string]$Scenario = 'Connecting',
    [int[]]$ProcessIds = @(),
    [ValidateRange(5,120)][int]$DurationSeconds = 25,
    [ValidateRange(250,2000)][int]$IntervalMs = 500
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$outputDir = Join-Path $repoRoot 'out\simnext-interop'
[IO.Directory]::CreateDirectory($outputDir) | Out-Null
$saltPath = Join-Path $outputDir 'local-hash-salt.txt'

if (-not (Test-Path -LiteralPath $saltPath -PathType Leaf)) {
    $random = [Security.Cryptography.RandomNumberGenerator]::Create()
    try {
        $bytes = New-Object byte[] 32
        $random.GetBytes($bytes)
        [IO.File]::WriteAllText($saltPath, [Convert]::ToBase64String($bytes))
    }
    finally { $random.Dispose() }
}
$script:localSalt = [Convert]::FromBase64String(
    [IO.File]::ReadAllText($saltPath).Trim())
if ($script:localSalt.Length -ne 32) {
    throw "Invalid local hash salt: $saltPath"
}
$script:localHash = [Security.Cryptography.SHA256]::Create()

function Get-EndpointTag([string]$RemoteIp) {
    # Tag for cross-run comparisons; original remote address not retained.
    if ([string]::IsNullOrWhiteSpace($RemoteIp)) { return '' }
    $ipBytes = [Text.Encoding]::UTF8.GetBytes($RemoteIp)
    $inputBytes = New-Object byte[] ($script:localSalt.Length + $ipBytes.Length)
    [Array]::Copy($script:localSalt, 0, $inputBytes, 0, $script:localSalt.Length)
    [Array]::Copy($ipBytes, 0, $inputBytes, $script:localSalt.Length, $ipBytes.Length)
    $digest = $script:localHash.ComputeHash($inputBytes)
    return [BitConverter]::ToString($digest).Replace('-', '').Substring(0, 16)
}

try {
    $processes = @(Get-CimInstance Win32_Process)
    if ($ProcessIds.Count -gt 0) {
        $roots = @($processes | Where-Object {
            $ProcessIds -contains [int]$_.ProcessId
        })
        if ($roots.Count -ne $ProcessIds.Count) {
            throw 'A PID informada nao existe. Confira no Gerenciador de Tarefas.'
        }
    }
    else {
        $roots = @($processes | Where-Object {
            $_.Name -match '(?i)sim.?next|intelbras.*(sim|client)'
        })
    }
    if ($roots.Count -eq 0) {
        Write-Host 'SIM Next nao localizado automaticamente.' -ForegroundColor Yellow
        Write-Host 'Abra o SIM Next e consulte os PIDs com:'
        Write-Host '  Get-Process | Where-Object { $_.ProcessName -match "sim|intelbras" } | Select-Object Id,ProcessName'
        throw 'Execute novamente com -ProcessIds PID[,PID].'
    }

    # Include child processes known at start. Do not inspect command lines.
    $ids = New-Object 'System.Collections.Generic.HashSet[int]'
    foreach ($p in $roots) { [void]$ids.Add([int]$p.ProcessId) }
    do {
        $changed = $false
        foreach ($p in $processes) {
            if ($ids.Contains([int]$p.ParentProcessId) -and
                $ids.Add([int]$p.ProcessId)) { $changed = $true }
        }
    } while ($changed)
    $idsArray = @($ids | Sort-Object)
    $metadata = @(
        foreach ($p in $processes) {
            if (-not $ids.Contains([int]$p.ProcessId)) { continue }
            $pidValue = [int]$p.ProcessId
            $signature = 'Unavailable'
            $exeVersion = ''
            $exeHash = ''
            $moduleNames = @()
            if ($p.ExecutablePath -and
                (Test-Path -LiteralPath $p.ExecutablePath -PathType Leaf)) {
                try {
                    $signature = [string](Get-AuthenticodeSignature -FilePath $p.ExecutablePath).Status
                    $exeVersion = [string](Get-Item -LiteralPath $p.ExecutablePath).VersionInfo.FileVersion
                    $exeHash = [string](Get-FileHash -LiteralPath $p.ExecutablePath -Algorithm SHA256).Hash
                }
                catch { $signature = 'Unreadable' }
            }
            try {
                $moduleNames = @((Get-Process -Id $pidValue -ErrorAction Stop).Modules |
                    ForEach-Object { $_.ModuleName } | Sort-Object -Unique)
            }
            catch { $moduleNames = @('[Modules not readable at current privileges]') }
            [pscustomobject]@{
                pid = $pidValue
                parentPid = [int]$p.ParentProcessId
                processName = [string]$p.Name
                signatureStatus = $signature
                fileVersion = $exeVersion
                exeSha256 = $exeHash
                loadedModuleNames = $moduleNames
            }
        }
    )

    Write-Host ''
    Write-Host "Cenario: $Scenario | PIDs: $($idsArray -join ', ')" -ForegroundColor Cyan
    Write-Host 'Este script apenas observa metadados; NAO captura pacotes ou credenciais.'
    Write-Host 'O cenario P2P/Relay deve ser identificado na INTERFACE do SIM Next.'
    [void](Read-Host 'Deixe o SIM Next pronto; pressione ENTER e execute a acao do cenario')

    $observations = New-Object 'System.Collections.Generic.List[object]'
    $watch = [Diagnostics.Stopwatch]::StartNew()
    while ($watch.Elapsed.TotalSeconds -lt $DurationSeconds) {
        $moment = [Math]::Round($watch.Elapsed.TotalSeconds, 3)
        try {
            $tcp = @(Get-NetTCPConnection -ErrorAction Stop)
            foreach ($conn in $tcp) {
                if (-not $ids.Contains([int]$conn.OwningProcess)) { continue }
                $observations.Add([pscustomobject]@{
                    sec = $moment
                    protocol = 'TCP'
                    pid = [int]$conn.OwningProcess
                    localPort = [int]$conn.LocalPort
                    remoteTag = Get-EndpointTag([string]$conn.RemoteAddress)
                    remotePort = [int]$conn.RemotePort
                    state = [string]$conn.State
                })
            }
            $udp = @(Get-NetUDPEndpoint -ErrorAction Stop)
            foreach ($ep in $udp) {
                if (-not $ids.Contains([int]$ep.OwningProcess)) { continue }
                $observations.Add([pscustomobject]@{
                    sec = $moment
                    protocol = 'UDP'
                    pid = [int]$ep.OwningProcess
                    localPort = [int]$ep.LocalPort
                    remoteTag = ''
                    remotePort = 0
                    state = 'UDP peer not exposed by Get-NetUDPEndpoint'
                })
            }
        }
        catch {
            Write-Warning ("Falha na amostragem: " + $_.Exception.Message)
        }
        Start-Sleep -Milliseconds $IntervalMs
    }
    $watch.Stop()
    $series = @($observations.ToArray())
    $grouped = @(
        $series | Group-Object protocol,pid,localPort,remoteTag,remotePort,state |
            ForEach-Object {
                $sample = $_.Group[0]
                [pscustomobject]@{
                    protocol = $sample.protocol
                    pid = $sample.pid
                    localPort = $sample.localPort
                    remoteTag = $sample.remoteTag
                    remotePort = $sample.remotePort
                    state = $sample.state
                    samples = $_.Count
                    firstSeenSeconds = ($_.Group | Measure-Object sec -Minimum).Minimum
                    lastSeenSeconds = ($_.Group | Measure-Object sec -Maximum).Maximum
                }
            } | Sort-Object protocol,pid,localPort,remoteTag
    )
    $report = [ordered]@{
        schema = 'sentrium.simnext.passive-metadata.v1'
        scenario = $Scenario
        generatedUtc = [DateTime]::UtcNow.ToString('o')
        durationSeconds = [Math]::Round($watch.Elapsed.TotalSeconds, 2)
        intervalMilliseconds = $IntervalMs
        importantLimit = 'UDP remote peer unavailable from Get-NetUDPEndpoint; use local Wireshark trace.'
        processes = $metadata
        connectionSummary = $grouped
    }
    $name = 'simnext-{0}-{1}.json' -f $Scenario.ToLowerInvariant(), [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')
    $path = Join-Path $outputDir $name
    ($report | ConvertTo-Json -Depth 12) |
        Set-Content -LiteralPath $path -Encoding UTF8

    Write-Host ''
    Write-Host "Relatorio local: $path" -ForegroundColor Green
    Write-Host "Processos: $($metadata.Count) | conexoes/portas unicas: $($grouped.Count)"
    Write-Host 'Nao compartilhe PCAPs brutos, senhas, tokens ou local-hash-salt.txt.'
    Write-Host 'Destinos UDP e negociacao P2P/Relay exigem captura Wireshark separada.'
}
finally { $script:localHash.Dispose() }
