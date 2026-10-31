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
    # Python launcher is missing/broken. Use local installed TShark for UDP only.
    # This fallback is intentionally PARTIAL, never claim the full RE-004 PASS.
    $tshark = Join-Path $env:ProgramFiles 'Wireshark\tshark.exe'
    if (-not (Test-Path -LiteralPath $tshark -PathType Leaf)) {
        $cmd = Get-Command tshark -ErrorAction SilentlyContinue
        if ($null -eq $cmd -or $cmd.CommandType -ne 'Application') {
            throw 'RE-004: Python e TShark ausentes; nenhum PASS declarado.'
        }
        $tshark = $cmd.Source
    }
    if ((Get-Item -LiteralPath $resolved).Length -gt 268435456) {
        throw 'RE-004: PCAPNG maior que 256 MiB; nenhum PASS declarado.'
    }
    $flows = @{}
    [long]$udpRead = 0
    $invalidRows = 0
    # Fields are PIPE-to-memory only. IPs never go to JSON or disk.
    & $tshark -r $resolved -n -Y 'udp && ip' -T fields -E 'separator=/t' -E 'occurrence=f' -e frame.len -e ip.src -e ip.dst -e udp.srcport -e udp.dstport | ForEach-Object {
        $line = ([string]$_).Split([char]9)
        if ($line.Length -ne 5) { $invalidRows++; return }
        try {
            $wire = [long]::Parse($line[0], [Globalization.CultureInfo]::InvariantCulture)
            $src = $line[1]; $dst = $line[2]
            $sport = [int]::Parse($line[3]); $dport = [int]::Parse($line[4])
            if (-not $src -or -not $dst -or $wire -le 0) { throw 'invalid' }
        } catch { $invalidRows++; return }
        $udpRead++
        $ep1 = '[' + $src + ']:' + $sport
        $ep2 = '[' + $dst + ']:' + $dport
        if ([string]::CompareOrdinal($ep1,$ep2) -gt 0) {
            $temp=$ep1; $ep1=$ep2; $ep2=$temp
            $temp=$src; $src=$dst; $dst=$temp
            $temp=$sport; $sport=$dport; $dport=$temp
        }
        $key=$ep1 + '|' + $ep2
        if (-not $flows.ContainsKey($key)) {
            $flows[$key]=@{a=$src;b=$dst;ap=$sport;bp=$dport;packets=[long]0;bytes=[long]0}
        }
        $flows[$key].packets++
        $flows[$key].bytes += $wire
    }
    $tsharkCode=$LASTEXITCODE
    if ($tsharkCode -ne 0 -or $invalidRows -gt 0 -or $flows.Count -eq 0) {
        throw 'RE-004: TShark falhou ou retornou linhas invalidas; nenhum PASS declarado.'
    }
    $largest=@($flows.Values | Sort-Object -Property @{Expression={$_.bytes};Descending=$true})[0]
    $localPort=$null; $remotePort=$null
    if ($largest.a -match '^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[01])\.)') {
        $localPort=$largest.ap; $remotePort=$largest.bp
    } elseif ($largest.b -match '^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[01])\.)') {
        $localPort=$largest.bp; $remotePort=$largest.ap
    }
    $outputDirectory = Join-Path $root 'out\simnext-interop'
    [IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
    $output = Join-Path $outputDirectory ('re004-tshark-transport-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff') + '.json')
    $safe = [ordered]@{
        schema='sentrium.f005.pcap.tshark-transport.v1'
        status='PASS_PARTIAL_TRANSPORT_ONLY'
        privacy='No IP addresses, packets or secrets serialized'
        udpIpv4Packets=[long]$udpRead
        udpConversations=$flows.Count
        dominantUdp=[ordered]@{localPort=$localPort;remotePort=$remotePort;packets=[long]$largest.packets;frameBytes=[long]$largest.bytes}
        limitations=@('Only UDP/IPv4 sizes and ports. No DNS, UDP envelope, account authentication or Cloud bootstrap was checked.')
    }
    $safe | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $output -Encoding UTF8
    if (-not (Test-Path -LiteralPath $output -PathType Leaf)) {
        throw 'RE-004: TShark nao escreveu o relatorio.'
    }
    Write-Host ('RE-004 TSHARK PASS PARCIAL: ' + $largest.packets + ' pacotes / ' + $largest.bytes + ' bytes na maior conversa UDP.') -ForegroundColor Green
    Write-Host ('Relatorio sanitizado: ' + $output)
    Write-Host 'AVISO: a auditoria Python de DNS, framing e Cloud NAO foi executada.' -ForegroundColor Yellow
    return
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
