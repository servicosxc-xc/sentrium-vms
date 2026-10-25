param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,
    [string]$EvidenceDirectory = ""
)

$ErrorActionPreference = 'Stop'

function Assert-Sentrium([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FFMPEG PROBE FAIL: $Message" }
}

function Resolve-SentriumExecutable([string]$Root, [string]$Name) {
    foreach ($candidate in @((Join-Path $Root "bin\$Name"), (Join-Path $Root $Name))) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [IO.Path]::GetFullPath($candidate)
        }
    }
    throw "FFMPEG PROBE FAIL: $Name not found under SENTRIUM_FFMPEG_ROOT"
}

function Get-SentriumPeMachine([string]$Path) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    Assert-Sentrium ($bytes.Length -ge 64) "PE header too small: $Path"
    $pe = [BitConverter]::ToInt32($bytes, 60)
    Assert-Sentrium ($pe -ge 0 -and ($pe + 6) -le $bytes.Length) "PE offset invalid: $Path"
    Assert-Sentrium ([BitConverter]::ToUInt32($bytes, $pe) -eq 0x00004550) "PE signature invalid: $Path"
    return [BitConverter]::ToUInt16($bytes, $pe + 4)
}

function Invoke-SentriumProcess([string]$Program, [string[]]$Arguments, [string]$StdoutPath, [string]$StderrPath) {
    Remove-Item -LiteralPath $StdoutPath, $StderrPath -Force -ErrorAction SilentlyContinue
    $process = Start-Process -FilePath $Program -ArgumentList $Arguments -NoNewWindow -Wait -PassThru -RedirectStandardOutput $StdoutPath -RedirectStandardError $StderrPath
    [ordered]@{
        ExitCode = $process.ExitCode
        Stdout = if (Test-Path -LiteralPath $StdoutPath) { Get-Content -LiteralPath $StdoutPath -Raw } else { "" }
        Stderr = if (Test-Path -LiteralPath $StderrPath) { Get-Content -LiteralPath $StderrPath -Raw } else { "" }
    }
}

Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_FFMPEG_ROOT)) 'SENTRIUM_FFMPEG_ROOT is not set'

$root = [IO.Path]::GetFullPath($env:SENTRIUM_FFMPEG_ROOT)
Assert-Sentrium (Test-Path -LiteralPath $root -PathType Container) 'SENTRIUM_FFMPEG_ROOT directory does not exist'

$resolvedInput = Resolve-Path -LiteralPath $InputPath -ErrorAction SilentlyContinue
Assert-Sentrium ($null -ne $resolvedInput) "input sample not found: $InputPath"
$input = $resolvedInput.ProviderPath
Assert-Sentrium (Test-Path -LiteralPath $input -PathType Leaf) "input sample not found: $input"

$ffprobe = Resolve-SentriumExecutable $root 'ffprobe.exe'
$ffmpeg = Resolve-SentriumExecutable $root 'ffmpeg.exe'

Assert-Sentrium ((Get-SentriumPeMachine $ffprobe) -eq 0x8664) 'ffprobe.exe is not AMD64'
Assert-Sentrium ((Get-SentriumPeMachine $ffmpeg) -eq 0x8664) 'ffmpeg.exe is not AMD64'

if ([string]::IsNullOrWhiteSpace($EvidenceDirectory)) {
    $EvidenceDirectory = Join-Path (Split-Path $input -Parent) 'ffmpeg-probe'
}
if ([IO.Path]::IsPathRooted($EvidenceDirectory)) {
    $evidence = [IO.Path]::GetFullPath($EvidenceDirectory)
} else {
    $evidence = Join-Path $PWD.ProviderPath $EvidenceDirectory
    $evidence = [IO.Path]::GetFullPath($evidence)
}
[IO.Directory]::CreateDirectory($evidence) | Out-Null

$ffprobeVersion = (& $ffprobe -version | Select-Object -First 1).Trim()
$ffmpegVersion = (& $ffmpeg -version | Select-Object -First 1).Trim()
Assert-Sentrium ($ffprobeVersion.StartsWith('ffprobe version 9.0.2')) 'ffprobe version must be 9.0.2'
Assert-Sentrium ($ffmpegVersion.StartsWith('ffmpeg version 9.0.2')) 'ffmpeg version must be 9.0.2'

$quotedInput = '"' + $input + '"'

$probeStdout = Join-Path $evidence 'ffprobe.stdout.json'
$probeStderr = Join-Path $evidence 'ffprobe.stderr.txt'
$softwareStdout = Join-Path $evidence 'software-decode.stdout.txt'
$softwareStderr = Join-Path $evidence 'software-decode.stderr.txt'
$d3dStdout = Join-Path $evidence 'd3d11va-decode.stdout.txt'
$d3dStderr = Join-Path $evidence 'd3d11va-decode.stderr.txt'

$probe = Invoke-SentriumProcess $ffprobe @('-hide_banner','-v','error','-show_format','-show_streams','-print_format','json',$quotedInput) $probeStdout $probeStderr

$probeJson = $null
$videoStreams = @()
$formatName = $null
$formatLongName = $null
if ($probe.ExitCode -eq 0 -and -not [string]::IsNullOrWhiteSpace($probe.Stdout)) {
    try {
        $probeJson = $probe.Stdout | ConvertFrom-Json
        $videoStreams = @($probeJson.streams | Where-Object { $_.codec_type -eq 'video' })
        if ($probeJson.format) {
            $formatName = $probeJson.format.format_name
            $formatLongName = $probeJson.format.format_long_name
        }
    } catch {
        $probeJson = $null
    }
}

$demuxSupported = ($probe.ExitCode -eq 0 -and $null -ne $probeJson -and $videoStreams.Count -gt 0)

$softwareDecision = 'NOT RUN'
$softwareExit = $null
if ($demuxSupported) {
    $software = Invoke-SentriumProcess $ffmpeg @('-hide_banner','-v','error','-nostdin','-benchmark','-i',$quotedInput,'-map','0:v:0','-an','-sn','-dn','-f','null','NUL') $softwareStdout $softwareStderr
    $softwareExit = $software.ExitCode
    $softwareDecision = if ($software.ExitCode -eq 0) { 'SUPPORTED' } else { 'UNSUPPORTED' }
} else {
    Set-Content -LiteralPath $softwareStderr -Value 'NOT RUN: ffprobe did not identify a decodable video stream without format forcing.' -Encoding UTF8
}

$d3dDecision = 'NOT APPLICABLE'
$d3dExit = $null
if ($softwareDecision -eq 'SUPPORTED') {
    $d3d = Invoke-SentriumProcess $ffmpeg @('-hide_banner','-v','error','-nostdin','-benchmark','-hwaccel','d3d11va','-i',$quotedInput,'-map','0:v:0','-an','-sn','-dn','-f','null','NUL') $d3dStdout $d3dStderr
    $d3dExit = $d3d.ExitCode
    $d3dDecision = if ($d3d.ExitCode -eq 0) { 'SUPPORTED' } else { 'UNSUPPORTED' }
} else {
    Set-Content -LiteralPath $d3dStderr -Value 'NOT APPLICABLE: D3D11VA probe requires successful FFmpeg software decode first.' -Encoding UTF8
}

$video = if ($videoStreams.Count -gt 0) { $videoStreams[0] } else { $null }

$result = [ordered]@{
    Probe = 'SENTRIUM-F004-FFMPEG'
    InputFileName = [IO.Path]::GetFileName($input)
    InputBytes = (Get-Item -LiteralPath $input).Length
    FFprobe = $ffprobeVersion
    FFmpeg = $ffmpegVersion
    FFprobeSha256 = (Get-FileHash -LiteralPath $ffprobe -Algorithm SHA256).Hash
    FFmpegSha256 = (Get-FileHash -LiteralPath $ffmpeg -Algorithm SHA256).Hash
    FFprobeExitCode = $probe.ExitCode
    DemuxDecision = if ($demuxSupported) { 'SUPPORTED' } else { 'UNSUPPORTED' }
    FormatName = $formatName
    FormatLongName = $formatLongName
    CodecName = if ($video) { $video.codec_name } else { $null }
    CodecLongName = if ($video) { $video.codec_long_name } else { $null }
    Width = if ($video) { $video.width } else { $null }
    Height = if ($video) { $video.height } else { $null }
    PixelFormat = if ($video) { $video.pix_fmt } else { $null }
    SoftwareDecodeDecision = $softwareDecision
    SoftwareDecodeExitCode = $softwareExit
    D3D11VADecision = $d3dDecision
    D3D11VAExitCode = $d3dExit
    EvidenceDirectory = $evidence
}

$resultPath = Join-Path $evidence 'result.json'
$result | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $resultPath -Encoding UTF8
$result | ConvertTo-Json -Depth 6
