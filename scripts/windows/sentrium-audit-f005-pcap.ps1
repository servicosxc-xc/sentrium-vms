<#
.SYNOPSIS
  RE-004: generate sanitized PCAPNG timing/UDP metadata on Windows.
.DESCRIPTION
  Read-only; Python >=3.9 standard library only. Reject stale py launchers and
  broken WindowsApps aliases instead of claiming PASS. No pip/network calls.
.EXAMPLE
  .\scripts\windows\sentrium-audit-f005-pcap.ps1 -PcapPath 'C:\Capturas\simnext.pcapng'
.EXAMPLE
  .\scripts\windows\sentrium-audit-f005-pcap.ps1 -PcapPath 'C:\Capturas\simnext.pcapng' -PythonPath 'C:\Python312\python.exe'
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PcapPath,
    [string]$PythonPath
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$analyzer = Join-Path $root 'scripts\research\sentrium-p2p-pcap-audit.py'
$resolved = (Resolve-Path -LiteralPath $PcapPath -ErrorAction Stop).ProviderPath
if (-not (Test-Path -LiteralPath $analyzer -PathType Leaf)) {
    throw 'RE-004: analisador Python nao encontrado no repositorio.'
}

function Test-PythonCandidate {
    param([string]$Executable, [string[]]$PrefixArgs)
    # Launcher py.exe can exist while its registered Python313\python.exe
    # no longer exists. A successful process launch and version are required.
    $probe = 'import sys; print("SENTRIUM_PY_OK" if sys.version_info >= (3, 9) else "SENTRIUM_PY_OLD")'
    try {
        $probeOutput = & $Executable @PrefixArgs -c $probe 2>&1
        $probeExit = $LASTEXITCODE
        return ($probeExit -eq 0 -and ((@($probeOutput) -join "`n") -match 'SENTRIUM_PY_OK'))
    }
    catch {
        return $false
    }
}

$candidates = New-Object 'System.Collections.Generic.List[object]'
if (-not [string]::IsNullOrWhiteSpace($PythonPath)) {
    $explicit = (Resolve-Path -LiteralPath $PythonPath -ErrorAction Stop).ProviderPath
    $candidates.Add([pscustomobject]@{ Exe = $explicit; Prefix = @() })
}
else {
    # Try default launcher first, but FALL BACK when it points to a deleted runtime.
    foreach ($commandName in @('py', 'python', 'python3')) {
        $commands = @(Get-Command -Name $commandName -All -ErrorAction SilentlyContinue |
            Where-Object { $_.CommandType -eq 'Application' })
        foreach ($command in $commands) {
            if ($command.Source -like '*\WindowsApps\python*.exe') { continue }
            $prefix = if ($commandName -eq 'py') { @('-3') } else { @() }
            $candidates.Add([pscustomobject]@{ Exe = $command.Source; Prefix = $prefix })
        }
    }
    # py -3 may choose a stale 3.13 registration while 3.12/3.11 still works.
    $launchers = @($candidates | Where-Object { $_.Exe -match '(?i)[\\/]py\.exe$' })
    foreach ($launcher in $launchers) {
        foreach ($v in @('-3.14', '-3.13', '-3.12', '-3.11', '-3.10', '-3.9')) {
            $candidates.Add([pscustomobject]@{ Exe = $launcher.Exe; Prefix = @($v) })
        }
    }
    # Check common per-user Python installation directories, even if PATH broke.
    if (-not [string]::IsNullOrWhiteSpace($env:LOCALAPPDATA)) {
        $perUserRoot = Join-Path $env:LOCALAPPDATA 'Programs\Python'
        if (Test-Path -LiteralPath $perUserRoot -PathType Container) {
            foreach ($dir in @(Get-ChildItem -LiteralPath $perUserRoot -Directory -ErrorAction SilentlyContinue)) {
                $exe = Join-Path $dir.FullName 'python.exe'
                if (Test-Path -LiteralPath $exe -PathType Leaf) {
                    $candidates.Add([pscustomobject]@{ Exe = $exe; Prefix = @() })
                }
            }
        }
    }
}

$chosen = $null
foreach ($candidate in $candidates) {
    if (Test-PythonCandidate -Executable $candidate.Exe -PrefixArgs $candidate.Prefix) {
        $chosen = $candidate
        break
    }
}
if ($null -eq $chosen) {
    throw ('RE-004: Python 3.9+ funcional nao encontrado. O launcher py pode apontar ' +
        'para Python313 removido. Verifique: py -0p; where.exe python; ' +
        'Get-Command python -All. Repare ou reinstale Python 3, ou informe ' +
        '-PythonPath com o caminho real do python.exe. Nenhum resultado foi declarado PASS.')
}

$outputDirectory = Join-Path $root 'out\simnext-interop'
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
$output = Join-Path $outputDirectory ('re004-sanitized-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff') + '.json')
$exe = $chosen.Exe
$prefix = @($chosen.Prefix)
Write-Host ('RE-004: Python validado: ' + $exe + ' ' + ($prefix -join ' '))
try {
    & $exe @prefix $analyzer $resolved --output $output
    $analysisExit = $LASTEXITCODE
}
catch {
    throw ('RE-004: nao foi possivel executar o analisador Python: ' + $_.Exception.Message)
}
if ($analysisExit -ne 0 -or -not (Test-Path -LiteralPath $output -PathType Leaf)) {
    throw 'RE-004: a analise nao concluiu; nenhum resultado foi declarado PASS.'
}
Write-Host 'RE-004 PASS: relatorio de metadados sanitizados (sem IPs, seriais, payloads):' -ForegroundColor Green
Write-Host $output
Write-Host 'O PCAPNG original permanece local, inalterado. Revise o JSON antes de compartilhar.'
