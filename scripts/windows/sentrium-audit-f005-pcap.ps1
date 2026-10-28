<#
.SYNOPSIS
  RE-004: produce sanitized packet timing / UDP envelope statistics from local PCAPNG.
.DESCRIPTION
  Read-only research tool. Requires Python 3 standard library (py -3 or python).
  PCAP content, device serials, remote IP addresses, and credentials are NEVER
  copied into Git or included in the generated report.
.EXAMPLE
  .\scripts\windows\sentrium-audit-f005-pcap.ps1 -PcapPath 'C:\Capturas\simnext.pcapng'
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PcapPath
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$analyzer = Join-Path $root 'scripts\research\sentrium-p2p-pcap-audit.py'
$resolved = (Resolve-Path -LiteralPath $PcapPath -ErrorAction Stop).ProviderPath
$outputDirectory = Join-Path $root 'out\simnext-interop'
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
$output = Join-Path $outputDirectory ('re004-sanitized-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss') + '.json')

# Detect a real Python interpreter; no pip installation or network requests.
$command = Get-Command py -ErrorAction SilentlyContinue
if ($null -ne $command) {
    & $command.Source -3 $analyzer $resolved --output $output
}
else {
    $command = Get-Command python -ErrorAction SilentlyContinue
    if ($null -eq $command) {
        throw 'Python 3 nao foi encontrado. E necessario py -3 ou python na PATH; nao instale pacotes.'
    }
    & $command.Source $analyzer $resolved --output $output
}
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $output)) {
    throw 'RE-004: a analise nao concluiu; nenhum resultado foi declarado PASS.'
}
Write-Host 'RE-004 PASS: relatorio de metadados sanitizados (sem IPs, seriais, payloads):' -ForegroundColor Green
Write-Host $output
Write-Host 'O arquivo PCAPNG original permanece local, inalterado. Revise o JSON antes de compartilhar.'
