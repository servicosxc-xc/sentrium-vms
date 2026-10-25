param([ValidateSet('Debug','Release','ASan')][string[]]$Configuration = @('Debug','Release'),[string]$RunName = 'sentrium-validation')
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ($RunName -notmatch '^sentrium-[a-z0-9-]+$') { throw 'Invalid evidence run name' }
$evidence = Join-Path $root "docs\foundation-001\evidence\$RunName"
if (Test-Path -LiteralPath $evidence) { throw 'Evidence already exists; select a new RunName to preserve history' }
. (Join-Path $PSScriptRoot 'sentrium-preflight.ps1') -EvidenceDirectory $evidence
$sourceFiles = @(Get-Item -LiteralPath (Join-Path $root 'CMakeLists.txt'),(Join-Path $root 'CMakePresets.json'),(Join-Path $root 'vcpkg.json'); Get-ChildItem -LiteralPath (Join-Path $root 'apps'),(Join-Path $root 'libs'),(Join-Path $root 'tests'),(Join-Path $root 'cmake'),(Join-Path $root 'scripts') -Recurse -File)
$inventory = @($sourceFiles | Sort-Object FullName | ForEach-Object { [ordered]@{Path=$_.FullName.Substring($root.Length+1).Replace('\','/');SHA256=(Get-FileHash -LiteralPath $_.FullName).Hash} })
$identityText = $inventory | ConvertTo-Json -Compress
$sha = [Security.Cryptography.SHA256]::Create()
try { $digest = [BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($identityText))).Replace('-','').ToLowerInvariant() } finally { $sha.Dispose() }
$buildId = 'source-' + $digest.Substring(0,16)
$inventory | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence 'source-inventory.json') -Encoding UTF8
$buildId | Set-Content -LiteralPath (Join-Path $evidence 'build-id.txt') -Encoding UTF8
function Invoke-Recorded([string]$Step,[string]$Program,[string[]]$Arguments) {
    [ordered]@{Program=$Program;Arguments=$Arguments;WorkingDirectory=$root} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence "$Step-command.json") -Encoding UTF8
    $savedPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Program @Arguments 2>&1 | Tee-Object -FilePath (Join-Path $evidence "$Step.log") | Out-Null
        $code = $LASTEXITCODE
    } finally { $ErrorActionPreference = $savedPreference }
    $code | Set-Content -LiteralPath (Join-Path $evidence "$Step-exit.txt")
    Write-Host "$Step exit=$code (complete output preserved)"
    if ($code -ne 0) { Get-Content -LiteralPath (Join-Path $evidence "$Step.log") -Tail 30 | Write-Host }
    if ($code -ne 0) { throw "Gate failed: $Step exit $code; evidence preserved" }
}
Push-Location $root
try {
    foreach ($config in $Configuration) {
        $preset = 'sentrium-' + $config.ToLowerInvariant()
        $build = Join-Path $root "out\$preset"
        if (Test-Path -LiteralPath $build) { throw "Clean build required; existing output preserved: $build" }
        $temp = Join-Path $root 'out\sentrium-temp'
        [IO.Directory]::CreateDirectory($temp) | Out-Null
        $env:TEMP=$temp; $env:TMP=$temp
        Invoke-Recorded "$preset-configure" $env:SENTRIUM_CMAKE @('--preset',$preset,"-DSENTRIUM_BUILD_ID=$buildId")
        Invoke-Recorded "$preset-build" $env:SENTRIUM_CMAKE @('--build','--preset',$preset,'--verbose')
        $ctest = Join-Path (Split-Path $env:SENTRIUM_CMAKE -Parent) 'ctest.exe'
        Invoke-Recorded "$preset-test" $ctest @('--preset',$preset,'--verbose','--output-junit',(Join-Path $evidence "$preset-junit.xml"))
        Copy-Item -LiteralPath (Join-Path $build 'Testing\Temporary\LastTest.log') -Destination (Join-Path $evidence "$preset-LastTest.log")
        $dumpbin = Join-Path (Split-Path $env:SENTRIUM_CL -Parent) 'dumpbin.exe'
        Invoke-Recorded "$preset-imports" $dumpbin @('/imports',(Join-Path $build 'sentrium-server.exe'))
        Invoke-Recorded "$preset-headers" $dumpbin @('/headers',(Join-Path $build 'sentrium-server.exe'))
        Get-ChildItem -LiteralPath $build -Filter '*.exe' | Get-FileHash | Select-Object Path,Hash | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence "$preset-binary-hashes.json") -Encoding UTF8
    }
} finally { Pop-Location }
Write-Host "Build and test gates passed. Evidence: $evidence"
