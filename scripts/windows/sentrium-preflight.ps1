param([switch]$VerifyOnly, [string]$EvidenceDirectory)
$ErrorActionPreference = 'Stop'
$sentriumRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$toolRoot = Join-Path (Split-Path $sentriumRoot -Parent) '_toolchain'
function Assert-Sentrium([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "PREFLIGHT FAIL: $Message" }
}
function Invoke-SentriumRead([string]$Program, [string[]]$Arguments) {
    $lines = & $Program @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) { throw "PREFLIGHT FAIL: $Program exit $LASTEXITCODE" }
    return ($lines -join "`n").Trim()
}
function Assert-SentriumPeAmd64([string]$Path, [string]$Label) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    Assert-Sentrium ($bytes.Length -ge 64) "$Label PE header too small"
    $pe = [BitConverter]::ToInt32($bytes,60)
    Assert-Sentrium ($pe -ge 0 -and ($pe + 6) -le $bytes.Length) "$Label PE offset invalid"
    Assert-Sentrium ([BitConverter]::ToUInt32($bytes,$pe) -eq 0x00004550) "$Label PE signature invalid"
    Assert-Sentrium ([BitConverter]::ToUInt16($bytes,$pe+4) -eq 0x8664) "$Label is not AMD64"
}
Assert-Sentrium ($sentriumRoot -eq 'C:\Projetos\SENTRIUM VMS\sentrium-vms') 'Unauthorized workspace'
foreach ($path in @($sentriumRoot,(Split-Path $sentriumRoot -Parent),'C:\Projetos')) {
    Assert-Sentrium (-not ((Get-Item -LiteralPath $path).Attributes -band [IO.FileAttributes]::ReparsePoint)) 'Workspace reparse point'
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$rawVs = Invoke-SentriumRead $vswhere @('-all','-products','Microsoft.VisualStudio.Product.BuildTools','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-format','json','-utf8')
$allInstances = ConvertFrom-Json -InputObject $rawVs
$instances = @($allInstances | Where-Object { $_.installationVersion -eq '17.14.37710.0' -and $_.isComplete -and $_.isLaunchable })
Assert-Sentrium ($instances.Count -eq 1) 'Exact complete Build Tools instance not uniquely found'
$vs = $instances[0]
Assert-Sentrium ($vs.catalog.productSemanticVersion.StartsWith('17.14.41+')) 'VS product version mismatch'
$devcmd = Join-Path $vs.installationPath 'Common7\Tools\VsDevCmd.bat'
$command = '"' + $devcmd + '" -no_logo -arch=x64 -host_arch=x64 -vcvars_ver=14.44.35207 -winsdk=10.0.26100.0 && set'
$environment = & $env:ComSpec /d /s /c $command
Assert-Sentrium ($LASTEXITCODE -eq 0) 'Official MSVC initialization failed'
foreach ($line in $environment) {
    if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process') }
}
Assert-Sentrium ($env:VSCMD_ARG_HOST_ARCH -eq 'x64' -and $env:VSCMD_ARG_TGT_ARCH -eq 'x64') 'Host/target mismatch'
Assert-Sentrium ($env:VCToolsVersion -eq '14.44.35207') 'VCToolsVersion mismatch'
$env:SENTRIUM_CL = (Get-Command cl.exe).Source
$env:SENTRIUM_LINK = (Get-Command link.exe).Source
$expectedBin = Join-Path $vs.installationPath 'VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64'
Assert-Sentrium ($env:SENTRIUM_CL -eq (Join-Path $expectedBin 'cl.exe')) 'Compiler origin mismatch'
Assert-Sentrium ($env:SENTRIUM_LINK -eq (Join-Path $expectedBin 'link.exe')) 'Linker origin mismatch'
Assert-Sentrium ((Get-Item -LiteralPath $env:SENTRIUM_CL).VersionInfo.FileVersion -eq '19.44.35229.0') 'Compiler version mismatch'
Assert-Sentrium ((Get-Item -LiteralPath $env:SENTRIUM_LINK).VersionInfo.FileVersion -eq '14.44.35229.0') 'Linker version mismatch'
Assert-Sentrium (Test-Path -LiteralPath (Join-Path $vs.installationPath 'MSBuild\Microsoft\VC\v170\Platforms\x64\PlatformToolsets\v143')) 'Platform toolset v143 missing'
foreach ($binary in @($env:SENTRIUM_CL,$env:SENTRIUM_LINK)) {
    $bytes = [IO.File]::ReadAllBytes($binary)
    $pe = [BitConverter]::ToInt32($bytes,60)
    Assert-Sentrium ([BitConverter]::ToUInt16($bytes,$pe+4) -eq 0x8664) 'Host PE not AMD64'
}
Assert-Sentrium ($env:WindowsSDKVersion -eq '10.0.26100.0\') 'SDK directory version mismatch'
$rc = Join-Path $env:WindowsSdkDir 'bin\10.0.26100.0\x64\rc.exe'
Assert-Sentrium ((Get-Item -LiteralPath $rc).VersionInfo.ProductVersion -eq '10.0.26100.9457') 'SDK servicing revision mismatch'
# Uninstall registry keys do not all expose DisplayName/DisplayVersion.
# Access them through PSObject to remain compatible with Set-StrictMode.
$sdkPackages = @(
    Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*',
                     'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*' -ErrorAction SilentlyContinue |
        Where-Object {
            $nameProperty = $_.PSObject.Properties['DisplayName']
            $versionProperty = $_.PSObject.Properties['DisplayVersion']
            $null -ne $nameProperty -and $null -ne $versionProperty -and
                [string]$nameProperty.Value -match 'Windows SDK|Windows Software Development Kit' -and
                [string]$versionProperty.Value -eq '10.1.26100.9457'
        } |
        Select-Object DisplayName, DisplayVersion
)
Assert-Sentrium ($sdkPackages.Count -gt 0) 'SDK package identity unavailable'
foreach ($relative in @('Include\10.0.26100.0\um\Windows.h','Lib\10.0.26100.0\um\x64\kernel32.lib','Lib\10.0.26100.0\ucrt\x64\ucrt.lib')) {
    Assert-Sentrium (Test-Path -LiteralPath (Join-Path $env:WindowsSdkDir $relative)) 'SDK header/lib missing'
}
$env:SENTRIUM_CMAKE = Join-Path $toolRoot 'cmake-4.4.4\bin\cmake.exe'
$env:SENTRIUM_NINJA = Join-Path $toolRoot 'ninja-1.13.2\ninja.exe'
$env:SENTRIUM_VCPKG_ROOT = Join-Path $toolRoot 'vcpkg'
$cmakeVersion = Invoke-SentriumRead $env:SENTRIUM_CMAKE @('--version')
$ninjaVersion = Invoke-SentriumRead $env:SENTRIUM_NINJA @('--version')
Assert-Sentrium ($cmakeVersion.Split("`n")[0] -eq 'cmake version 4.4.4') 'CMake mismatch'
Assert-Sentrium ($ninjaVersion -eq '1.13.2') 'Ninja mismatch'

Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_QT_ROOT)) 'SENTRIUM_QT_ROOT missing'
$qtRoot = [IO.Path]::GetFullPath($env:SENTRIUM_QT_ROOT)
Assert-Sentrium ($qtRoot -eq 'C:\Qt\6.12.0\msvc2022_64') 'Qt root mismatch'
foreach ($relative in @(
    'lib\cmake\Qt6\Qt6Config.cmake',
    'lib\cmake\Qt6Core\Qt6CoreConfig.cmake',
    'lib\cmake\Qt6Gui\Qt6GuiConfig.cmake',
    'lib\cmake\Qt6Qml\Qt6QmlConfig.cmake',
    'lib\cmake\Qt6Quick\Qt6QuickConfig.cmake',
    'lib\cmake\Qt6QuickControls2\Qt6QuickControls2Config.cmake',
    'qml\QtQuick\Controls',
    'plugins\platforms\qwindows.dll',
    'plugins\platforms\qwindowsd.dll',
    'bin\Qt6Core.dll',
    'bin\Qt6Cored.dll',
    'bin\Qt6Gui.dll',
    'bin\Qt6Guid.dll',
    'bin\Qt6Qml.dll',
    'bin\Qt6Qmld.dll',
    'bin\Qt6Quick.dll',
    'bin\Qt6Quickd.dll',
    'bin\qmake.exe'
)) {
    Assert-Sentrium (Test-Path -LiteralPath (Join-Path $qtRoot $relative)) "Qt component missing: $relative"
}
$qtVersion = Invoke-SentriumRead (Join-Path $qtRoot 'bin\qmake.exe') @('-query','QT_VERSION')
$qtPrefix = Invoke-SentriumRead (Join-Path $qtRoot 'bin\qmake.exe') @('-query','QT_INSTALL_PREFIX')
$qtSpec = Invoke-SentriumRead (Join-Path $qtRoot 'bin\qmake.exe') @('-query','QMAKE_SPEC')
Assert-Sentrium ($qtVersion -eq '6.12.0') 'Qt version mismatch'
Assert-Sentrium ($qtPrefix.Replace('/','\') -eq $qtRoot) 'Qt prefix mismatch'
Assert-Sentrium ($qtSpec -eq 'win32-msvc') 'Qt compiler spec mismatch'
Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_INTELBRAS_SDK_ROOT)) 'SENTRIUM_INTELBRAS_SDK_ROOT missing'
$sdkRoot = [IO.Path]::GetFullPath($env:SENTRIUM_INTELBRAS_SDK_ROOT)
Assert-Sentrium (Test-Path -LiteralPath $sdkRoot -PathType Container) 'Intelbras NetSDK root unavailable'
$repoPrefix = $sentriumRoot.TrimEnd('\') + '\'
Assert-Sentrium (
    -not $sdkRoot.Equals($sentriumRoot,[StringComparison]::OrdinalIgnoreCase) -and
    -not $sdkRoot.StartsWith($repoPrefix,[StringComparison]::OrdinalIgnoreCase)
) 'Intelbras SDK must remain external to the repository'
$sdkRequired = @(
    'include\dhnetsdk.h',
    'bin\dhnetsdk.lib',
    'bin\dhconfigsdk.lib',
    'bin\dhnetsdk.dll',
    'bin\dhconfigsdk.dll'
)
foreach ($relative in $sdkRequired) {
    Assert-Sentrium (Test-Path -LiteralPath (Join-Path $sdkRoot $relative) -PathType Leaf) "Intelbras SDK component missing: $relative"
}
$dhnetsdkHeader = Join-Path $sdkRoot 'include\dhnetsdk.h'
$headerText = Get-Content -LiteralPath $dhnetsdkHeader -Raw
foreach ($symbol in @(
    'CLIENT_InitEx',
    'CLIENT_SetAutoReconnect',
    'CLIENT_LoginEx2',
    'CLIENT_GetLastError',
    'CLIENT_Logout',
    'CLIENT_Cleanup',
    'CLIENT_RealPlayEx',
    'CLIENT_SetRealDataCallBackEx2',
    'CLIENT_StopRealPlayEx',
    'EM_LOGIN_SPEC_CAP_TCP',
    'DH_RType_Realplay',
    'DH_RType_Realplay_1',
    'nChanNum'
)) {
    Assert-Sentrium ($headerText.Contains($symbol)) "Intelbras NetSDK header contract missing: $symbol"
}
Assert-SentriumPeAmd64 (Join-Path $sdkRoot 'bin\dhnetsdk.dll') 'dhnetsdk.dll'
Assert-SentriumPeAmd64 (Join-Path $sdkRoot 'bin\dhconfigsdk.dll') 'dhconfigsdk.dll'

Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_PLAYSDK_ROOT)) 'SENTRIUM_PLAYSDK_ROOT missing'
$playSdkRoot = [IO.Path]::GetFullPath($env:SENTRIUM_PLAYSDK_ROOT)
Assert-Sentrium (Test-Path -LiteralPath $playSdkRoot -PathType Container) 'Intelbras PlaySDK root unavailable'
Assert-Sentrium (
    -not $playSdkRoot.Equals($sentriumRoot,[StringComparison]::OrdinalIgnoreCase) -and
    -not $playSdkRoot.StartsWith($repoPrefix,[StringComparison]::OrdinalIgnoreCase)
) 'PlaySDK must remain external to the repository'
foreach ($relative in @('DLL\dhplay.h','DLL\dhplay.lib','DLL\dhplay.dll')) {
    Assert-Sentrium (Test-Path -LiteralPath (Join-Path $playSdkRoot $relative) -PathType Leaf) "PlaySDK component missing: $relative"
}
$dhplayHeader = Join-Path $playSdkRoot 'DLL\dhplay.h'
$playHeaderText = Get-Content -LiteralPath $dhplayHeader -Raw
foreach ($symbol in @(
    'PLAY_GetFreePort',
    'PLAY_ReleasePort',
    'PLAY_SetStreamOpenMode',
    'PLAY_OpenStream',
    'PLAY_SetDecCBStream',
    'PLAY_SetDecCallBackEx',
    'PLAY_Play',
    'PLAY_InputData',
    'PLAY_Stop',
    'PLAY_CloseStream',
    'PLAY_GetLastError',
    'STREAME_REALTIME',
    'FRAME_INFO'
)) {
    Assert-Sentrium ($playHeaderText.Contains($symbol)) "Intelbras PlaySDK header contract missing: $symbol"
}
Assert-SentriumPeAmd64 (Join-Path $playSdkRoot 'DLL\dhplay.dll') 'dhplay.dll'
$env:SENTRIUM_INTELBRAS_SDK_ROOT = $sdkRoot
$env:SENTRIUM_PLAYSDK_ROOT = $playSdkRoot

$manifest = Get-Content -LiteralPath (Join-Path $sentriumRoot 'vcpkg.json') -Raw | ConvertFrom-Json
$head = Invoke-SentriumRead 'git' @('-C',$env:SENTRIUM_VCPKG_ROOT,'rev-parse','HEAD')
$tag = Invoke-SentriumRead 'git' @('-C',$env:SENTRIUM_VCPKG_ROOT,'rev-parse','2026.10.16^{commit}')
$origin = Invoke-SentriumRead 'git' @('-C',$env:SENTRIUM_VCPKG_ROOT,'remote','get-url','origin')
$dirty = Invoke-SentriumRead 'git' @('--no-optional-locks','-C',$env:SENTRIUM_VCPKG_ROOT,'status','--porcelain')
Assert-Sentrium ($head -eq $manifest.'builtin-baseline' -and $head -eq $tag -and $head.Length -eq 40) 'Single vcpkg baseline/tag mismatch'
Assert-Sentrium ($origin -eq 'https://github.com/microsoft/vcpkg.git' -and $dirty -eq '') 'vcpkg identity/working tree mismatch'
Assert-Sentrium (-not (Test-Path -LiteralPath (Join-Path $sentriumRoot 'vcpkg-configuration.json'))) 'Unexpected second baseline configuration'
$vcpkgTool = Invoke-SentriumRead (Join-Path $env:SENTRIUM_VCPKG_ROOT 'vcpkg.exe') @('version','--disable-metrics')
Assert-Sentrium ($vcpkgTool.StartsWith('vcpkg package management program version 2026-09-26-')) 'vcpkg-tool mismatch'
if (-not $VerifyOnly -and $EvidenceDirectory) {
    [IO.Directory]::CreateDirectory($EvidenceDirectory) | Out-Null
    $rawVs | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'vswhere.json') -Encoding UTF8
    $sdkPackages | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'sdk-packages.json') -Encoding UTF8
    $environment | Where-Object { $_ -match '^(VSCMD|VCTools|VCINSTALLDIR|VSINSTALLDIR|WindowsSDK|WindowsSdk|INCLUDE=|LIB=|LIBPATH=|PATH=)' } | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'msvc-environment.txt') -Encoding UTF8
    $paths = @(
        $env:SENTRIUM_CL,
        $env:SENTRIUM_LINK,
        $rc,
        $env:SENTRIUM_CMAKE,
        $env:SENTRIUM_NINJA,
        (Join-Path $env:SENTRIUM_VCPKG_ROOT 'vcpkg.exe'),
        $dhnetsdkHeader,
        $dhplayHeader,
        (Join-Path $sdkRoot 'bin\dhnetsdk.lib'),
        (Join-Path $sdkRoot 'bin\dhconfigsdk.lib'),
        (Join-Path $playSdkRoot 'DLL\dhplay.lib'),
        (Join-Path $sdkRoot 'bin\dhnetsdk.dll'),
        (Join-Path $sdkRoot 'bin\dhconfigsdk.dll'),
        (Join-Path $playSdkRoot 'DLL\dhplay.dll')
    )
    $paths | ForEach-Object { Get-FileHash -LiteralPath $_ -Algorithm SHA256 } | Select-Object Path,Hash,Algorithm | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'toolchain-hashes.json') -Encoding UTF8
    [ordered]@{Result='PREFLIGHT PASS';Workspace=$sentriumRoot;VS=$vs.installationVersion;VSProduct=$vs.catalog.productDisplayVersion;Installation=$vs.installationPath;VCTools=$env:VCToolsVersion;Compiler=$env:SENTRIUM_CL;Linker=$env:SENTRIUM_LINK;Host=$env:VSCMD_ARG_HOST_ARCH;Target=$env:VSCMD_ARG_TGT_ARCH;SDK='10.0.26100.9457';SDKDirectories=$env:WindowsSDKVersion;CMake=$cmakeVersion;Ninja=$ninjaVersion;QtVersion=$qtVersion;QtRoot=$qtRoot;QtSpec=$qtSpec;IntelbrasSDKRoot=$sdkRoot;PlaySDKRoot=$playSdkRoot;PlaySDK='validated-x64';VcpkgCommit=$head;VcpkgTag=$tag;VcpkgOrigin=$origin;VcpkgTool=$vcpkgTool} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'preflight.json') -Encoding UTF8
}
Write-Host 'PREFLIGHT PASS: SENTRIUM Foundation-005 workspace; exact toolchain lock; Qt 6.12.0 MSVC; external NetSDK/PlaySDK x64; single vcpkg baseline.'
