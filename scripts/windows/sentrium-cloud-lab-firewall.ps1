# F005 standalone lab: inbound TCP BLOCK for one exact .NET apphost. No vendor DLLs.
# Prepare builds as ordinary user, requests UAC ONLY for firewall rule creation.
[CmdletBinding()]
param([ValidateSet('Prepare','Install','Status','Remove')][string]$Action='Status')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$project=Join-Path $root 'tools\simnext-cloud-lab\Sentrium.CloudLibLab.csproj'
$app=[IO.Path]::GetFullPath((Join-Path $root 'tools\simnext-cloud-lab\bin\Release\net10.0\Sentrium.CloudLibLab.exe'))
$name='SENTRIUM-F005-LAB-TCP-IN-BLOCK'
$description='SENTRIUM F005 TEMPORARY LAB inbound TCP block for one verified apphost.'
function Is-Admin {
    $principal=New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}
function Assert-NotRunning {
    if (Get-Process -Name 'Sentrium.CloudLibLab' -ErrorAction SilentlyContinue) {
        throw 'LAB FIREWALL: encerre primeiro o processo Sentrium.CloudLibLab.'
    }
}
function Assert-Profiles {
    $profiles=@(Get-NetFirewallProfile -PolicyStore ActiveStore -ErrorAction Stop)
    if ($profiles.Count -lt 3) { throw 'LAB FIREWALL: lista de perfis incompleta.' }
    foreach($profile in $profiles) {
        if ([string]$profile.Enabled -ne 'True' -or
            [string]$profile.AllowLocalFirewallRules -eq 'False') {
            throw ('LAB FIREWALL: perfil nao aplica a protecao: ' + $profile.Name)
        }
    }
}
function Get-ExpectedRule {
    $rules=@(Get-NetFirewallRule -Name $name -PolicyStore PersistentStore -ErrorAction SilentlyContinue)
    if ($rules.Count -eq 0) { return $null }
    if ($rules.Count -ne 1) { throw 'LAB FIREWALL: nome de regra duplicado.' }
    $rule=$rules[0]
    $apps=@($rule | Get-NetFirewallApplicationFilter -ErrorAction Stop)
    $ports=@($rule | Get-NetFirewallPortFilter -ErrorAction Stop)
    if ($apps.Count -ne 1 -or $ports.Count -ne 1 -or
        -not [string]::Equals([string]$apps[0].Program,$app,[StringComparison]::OrdinalIgnoreCase) -or
        [string]$rule.Direction -ne 'Inbound' -or [string]$rule.Action -ne 'Block' -or
        [string]$rule.Enabled -ne 'True' -or [string]$rule.Description -ne $description -or
        ([string]$ports[0].Protocol -notin @('TCP','6')) -or
        [string]$ports[0].LocalPort -ne 'Any') {
        throw 'LAB FIREWALL: regra existente nao pertence exatamente ao laboratorio; nao modificar.'
    }
    return $rule
}
function Assert-Active {
    Assert-Profiles
    if ($null -eq (Get-ExpectedRule)) { throw 'LAB FIREWALL: regra nao instalada.' }
    $active=@(Get-NetFirewallRule -Name $name -PolicyStore ActiveStore -ErrorAction SilentlyContinue)
    if ($active.Count -ne 1 -or [string]$active[0].Enabled -ne 'True' -or
        [string]$active[0].Direction -ne 'Inbound' -or
        [string]$active[0].Action -ne 'Block') {
        throw 'LAB FIREWALL: regra nao esta ativa; nenhuma seguranca afirmada.'
    }
    $apps=@($active[0] | Get-NetFirewallApplicationFilter -ErrorAction Stop)
    if ($apps.Count -ne 1 -or
        -not [string]::Equals([string]$apps[0].Program,$app,[StringComparison]::OrdinalIgnoreCase)) {
        throw 'LAB FIREWALL: executavel ativo nao corresponde; nao executar lab.'
    }
    Write-Host 'F005 FIREWALL STATUS PASS: TCP inbound BLOCK ativo somente no apphost LAB.' -ForegroundColor Green
    Write-Host 'AVISO: regra ativa NAO comprova que acesso da LAN esteja bloqueado; testar de outro PC.'
}
function Call-Admin([string]$operation) {
    $powershell=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
    if (-not (Test-Path -LiteralPath $powershell)) { throw 'LAB FIREWALL: Windows PowerShell ausente.' }
    $arguments='-NoProfile -File "' + $PSCommandPath + '" -Action ' + $operation
    $child=Start-Process -FilePath $powershell -ArgumentList $arguments -Verb RunAs -Wait -PassThru -ErrorAction Stop
    if ($child.ExitCode -ne 0) { throw ('LAB FIREWALL: UAC/operacao elevada falhou: ' + $operation) }
}
switch($Action) {
    'Prepare' {
        if (Is-Admin) { throw 'LAB FIREWALL: Prepare deve rodar SEM elevar a Developer PowerShell.' }
        Assert-NotRunning
        $dotnet=Get-Command dotnet -ErrorAction Stop
        & $dotnet.Source build $project --configuration Release --nologo -p:UseAppHost=true
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $app -PathType Leaf)) {
            throw 'LAB FIREWALL: falha de build/apphost; regra nao solicitada.'
        }
        Write-Host 'F005: build do LAB feito como usuario normal. UAC sera usado SO para regra BLOCK.'
        Call-Admin 'Install'
        Assert-Active
        Write-Host 'F005 PROTECT READY: regra criada. NENHUMA DLL Cloud foi executada.' -ForegroundColor Green
    }
    'Install' {
        if (-not (Is-Admin)) { throw 'LAB FIREWALL: Install requer Administrador.' }
        Assert-NotRunning
        if (-not (Test-Path -LiteralPath $app -PathType Leaf)) { throw 'LAB FIREWALL: EXE ausente.' }
        Assert-Profiles
        if ($null -eq (Get-ExpectedRule)) {
            $args=@{
                Name=$name
                DisplayName='SENTRIUM F005 LAB TEMP - BLOCK TCP inbound'
                Description=$description
                Direction='Inbound'
                Action='Block'
                Protocol='TCP'
                LocalPort='Any'
                Program=$app
                Profile='Any'
                Enabled='True'
                PolicyStore='PersistentStore'
                ErrorAction='Stop'
            }
            New-NetFirewallRule @args | Out-Null
            Write-Host 'F005 FIREWALL: nova regra BLOCK por programa criada.'
        } else {
            Write-Host 'F005 FIREWALL: regra propria ja instalada, sem duplicacao.'
        }
        Assert-Active
    }
    'Status' {
        if (-not (Test-Path -LiteralPath $app -PathType Leaf)) { throw 'LAB FIREWALL: EXE ausente.' }
        Assert-Active
    }
    'Remove' {
        if (-not (Is-Admin)) {
            Call-Admin 'Remove'
            if ($null -ne (Get-ExpectedRule)) { throw 'LAB FIREWALL: regra ainda presente.' }
            Write-Host 'F005 FIREWALL REMOVED: nenhuma regra LAB restante.' -ForegroundColor Green
            return
        }
        Assert-NotRunning
        $owned=Get-ExpectedRule
        if ($null -ne $owned) {
            $owned | Remove-NetFirewallRule -ErrorAction Stop
        }
        if ($null -ne (Get-ExpectedRule)) { throw 'LAB FIREWALL: falha ao remover regra LAB.' }
        Write-Host 'F005 FIREWALL REMOVED: regra exclusiva LAB removida.' -ForegroundColor Green
    }
}
