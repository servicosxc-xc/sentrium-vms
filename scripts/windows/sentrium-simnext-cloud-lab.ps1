# SENTRIUM Cloud library lab: references vendor DLLs in place; NEVER copy or distribute.
# Default Inspect is read-only (metadata only). Connect requires explicit typed consent.
[CmdletBinding()]
param(
    [ValidateSet('Inspect','Connect','IsolationProbe')][string]$Mode = 'Inspect',
    [string]$LibraryDirectory
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$project = Join-Path $root 'tools\simnext-cloud-lab\Sentrium.CloudLibLab.csproj'
if (-not (Test-Path -LiteralPath $project -PathType Leaf)) {
    throw 'LAB FAIL: projeto do experimento ausente.'
}
if ([string]::IsNullOrWhiteSpace($LibraryDirectory)) {
    Add-Type -AssemblyName System.Windows.Forms
    $chooser = New-Object System.Windows.Forms.OpenFileDialog
    $chooser.Title = 'Selecione SIMNext.P2PSDK.dll da instalacao original do SIM Next'
    $chooser.Filter = 'SIMNext.P2PSDK.dll|SIMNext.P2PSDK.dll'
    $chooser.CheckFileExists = $true
    if ($chooser.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        Write-Host 'LAB CANCELLED: nenhuma biblioteca selecionada.'
        return
    }
    $LibraryDirectory = Split-Path -Parent $chooser.FileName
}
$folder = (Resolve-Path -LiteralPath $LibraryDirectory -ErrorAction Stop).ProviderPath
$names = @('SIMNext.P2PSDK.dll', 'SIMNext.Core.dll','SIMNext.Model.dll','Libt2uWrapper.dll','libt2u.dll')
foreach ($name in $names) {
    $full = Join-Path $folder $name
    if (-not (Test-Path -LiteralPath $full -PathType Leaf)) {
        throw ("LAB BLOCKED: falta biblioteca local " + $name + "; nenhum teste de conexao.")
    }
    $auth = Get-AuthenticodeSignature -LiteralPath $full
    Write-Host ($name + ': assinatura=' + $auth.Status + '; arquivo local presente')
}
$cmd = Get-Command dotnet -ErrorAction SilentlyContinue
if ($null -eq $cmd -or $cmd.CommandType -ne 'Application') {
    throw 'LAB BLOCKED: dotnet nao encontrado. Nenhum teste de conexao foi realizado.'
}
$sdks = & $cmd.Source --list-sdks
if ($LASTEXITCODE -ne 0 -or -not (@($sdks) -match '^10\.')) {
    throw 'LAB BLOCKED: .NET SDK 10 nao encontrado. Nenhum teste de conexao.'
}
Write-Host ("SENTRIUM Cloud Library LAB: " + $Mode.ToUpperInvariant() + " / bibliotecas no local original.")
Write-Host 'Nao copiar DLLs; nao remover ou alterar arquivos da Intelbras.'
if ($Mode -eq 'IsolationProbe') {
    $fw = Join-Path $PSScriptRoot 'sentrium-cloud-lab-firewall.ps1'
    $app = Join-Path $root 'tools\simnext-cloud-lab\bin\Release\net10.0\Sentrium.CloudLibLab.exe'
    $who = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    if ($who.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'ISOLATION PROBE: use PowerShell NORMAL, nao elevado.'
    }
    & $fw -Action Status
    if (-not (Test-Path -LiteralPath $app -PathType Leaf)) {
        throw 'ISOLATION PROBE: EXE apphost nao encontrado.'
    }
    try {
        Write-Host 'ISOLATION PROBE: SEM login NetSDK ou credenciais do DVR.'
        & $app --mode isolation-probe --library-dir $folder
        $result = $LASTEXITCODE
    } finally {
        Write-Host 'ISOLATION PROBE: solicitando UAC para remover regra temporaria.'
        try { & $fw -Action Remove }
        catch {
            Write-Warning 'Regra de BLOCK pode permanecer. Execute firewall.ps1 -Action Remove apos encerrar LAB.'
        }
    }
    if ($result -ne 0) { throw ('ISOLATION PROBE: falhou (exit=' + $result + ').') }
    Write-Host 'ISOLATION PROBE COMPLETE; falta considerar o teste LAN externo.'
    return
}
& $cmd.Source run --project $project --configuration Release --no-launch-profile -- --mode $Mode.ToLowerInvariant() --library-dir $folder
$exit = $LASTEXITCODE
if ($exit -ne 0) { throw ("SENTRIUM Cloud Library LAB: NAO APROVADO (exit=" + $exit + ").") }
if ($Mode -eq 'Inspect') {
    Write-Host 'INSPECT PASS: bibliotecas identificadas sem tentativa de conexao.' -ForegroundColor Green
} else {
    Write-Host 'LAB encerrado; conferir separadamente uso/redistribuicao antes de qualquer integracao.' -ForegroundColor Yellow
}
