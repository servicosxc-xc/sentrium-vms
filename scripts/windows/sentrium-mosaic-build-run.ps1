param([switch]$BuildOnly)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$buildDir = Join-Path $root 'out\sentrium-release'
$cacheFile = Join-Path $buildDir 'CMakeCache.txt'
$cmake = Join-Path (Split-Path $root -Parent) '_toolchain\cmake-4.4.4\bin\cmake.exe'
if (-not (Test-Path -LiteralPath $cacheFile -PathType Leaf)) { throw 'MOSAIC: missing CMakeCache.txt' }
if (-not (Test-Path -LiteralPath $cmake -PathType Leaf)) { throw 'MOSAIC: missing approved CMake' }
if ($env:VSCMD_ARG_HOST_ARCH -ne 'x64' -or $env:VSCMD_ARG_TGT_ARCH -ne 'x64') {
    throw 'MOSAIC: open VS 2022 x64 Developer PowerShell without administrator elevation'
}
if (@(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue).Count -ne 0) {
    throw 'MOSAIC: close the running SENTRIUM VMS before building'
}
$env:SENTRIUM_QT_ROOT = 'C:\Qt\6.12.0\msvc2022_64'
$cache = Get-Content -LiteralPath $cacheFile
foreach ($name in @('SENTRIUM_INTELBRAS_SDK_ROOT','SENTRIUM_PLAYSDK_ROOT')) {
    $found = @($cache | Where-Object { $_ -match ('^' + $name + ':[^=]+=') } | Select-Object -First 1)
    if ($found.Count -ne 1) { throw ('MOSAIC: SDK root not found in cache: ' + $name) }
    $value = $found[0].Substring($found[0].IndexOf('=') + 1)
    if (-not (Test-Path -LiteralPath $value -PathType Container)) {
        throw ('MOSAIC: SDK directory missing: ' + $name)
    }
    [Environment]::SetEnvironmentVariable($name, $value, 'Process')
}
$env:PATH = "$env:SENTRIUM_QT_ROOT\bin;$env:SENTRIUM_INTELBRAS_SDK_ROOT\bin;$env:SENTRIUM_PLAYSDK_ROOT\DLL;$env:PATH"
$keys = @('GIT_CONFIG_COUNT','GIT_CONFIG_KEY_0','GIT_CONFIG_VALUE_0','GIT_CONFIG_KEY_1','GIT_CONFIG_VALUE_1')
$previous = @{}
foreach ($key in $keys) { $previous[$key] = [Environment]::GetEnvironmentVariable($key, 'Process') }
$log = Join-Path $buildDir 'sentrium-mosaic-build-last.log'
try {
    $env:GIT_CONFIG_COUNT = '2'
    $env:GIT_CONFIG_KEY_0 = 'safe.directory'
    $env:GIT_CONFIG_VALUE_0 = $root.Replace('\','/')
    $env:GIT_CONFIG_KEY_1 = 'safe.directory'
    $env:GIT_CONFIG_VALUE_1 = (Join-Path (Split-Path $root -Parent) '_toolchain\vcpkg').Replace('\','/')
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $cmake --build --preset sentrium-release --target sentrium-client 2>&1 |
            Out-File -LiteralPath $log -Encoding utf8
        $buildExit = $LASTEXITCODE
    } finally { $ErrorActionPreference = $oldPreference }
} finally {
    foreach ($key in $keys) {
        [Environment]::SetEnvironmentVariable($key, $previous[$key], 'Process')
    }
}
Write-Host '=== MOSAIC BUILD: last 45 lines ==='
Get-Content -LiteralPath $log -Tail 45 | ForEach-Object { Write-Host $_ }
if ($buildExit -ne 0) { throw ('MOSAIC BUILD FAIL: exit=' + $buildExit + '; log=' + $log) }
$found = @(Get-ChildItem -LiteralPath $buildDir -Filter 'sentrium-client.exe' -File -Recurse)
if ($found.Count -ne 1) { throw ('MOSAIC: expected one client executable; found ' + $found.Count) }
$exe = $found[0]
Write-Host ('MOSAIC BUILD PASS: ' + $exe.FullName) -ForegroundColor Green
if ($BuildOnly) { return }
$process = Start-Process -FilePath $exe.FullName -WorkingDirectory $exe.DirectoryName -PassThru -ErrorAction Stop
Start-Sleep -Seconds 3
$process.Refresh()
if ($process.HasExited) { throw ('MOSAIC CLIENT EXIT: code=' + $process.ExitCode) }
Write-Host ('MOSAIC CLIENT RUNNING: PID=' + $process.Id) -ForegroundColor Green
Write-Host 'Verify the GUI, AutoRegistration channels and live moving video in 1/4/9/16 layouts.'
