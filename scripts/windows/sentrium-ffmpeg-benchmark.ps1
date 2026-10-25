param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,
    [ValidateRange(0, 1000)]
    [int]$Loops = 19,
    [ValidateRange(2, 10)]
    [int]$Repetitions = 3,
    [string]$EvidenceDirectory = ""
)

$ErrorActionPreference = 'Stop'

function Assert-Sentrium([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FFMPEG BENCH FAIL: $Message" }
}

function Resolve-SentriumExecutable([string]$Root, [string]$Name) {
    foreach ($candidate in @((Join-Path $Root "bin\$Name"), (Join-Path $Root $Name))) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [IO.Path]::GetFullPath($candidate)
        }
    }
    throw "FFMPEG BENCH FAIL: $Name not found under SENTRIUM_FFMPEG_ROOT"
}

function Quote-SentriumArgument([string]$Value) {
    if ($Value -notmatch '[\s"]') { return $Value }
    return '"' + ($Value -replace '"', '\"') + '"'
}

function Invoke-SentriumDecode([string]$Backend, [string]$FFmpeg, [string]$InputFile, [int]$LoopCount) {
    $args = @('-hide_banner','-loglevel','error','-nostdin','-nostats','-stream_loop',$LoopCount.ToString())
    if ($Backend -eq 'D3D11VA') { $args += @('-hwaccel','d3d11va') }
    $args += @('-i',$InputFile,'-map','0:v:0','-an','-sn','-dn','-f','null','NUL','-progress','pipe:1')

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $FFmpeg
    $psi.Arguments = (($args | ForEach-Object { Quote-SentriumArgument ([string]$_) }) -join ' ')
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $psi
    $wall = [Diagnostics.Stopwatch]::StartNew()
    Assert-Sentrium ($process.Start()) 'failed to start ffmpeg'
    # Capture peak memory before process exit; counters are unreliable afterward.
    # Read both redirected pipes asynchronously to prevent output-buffer deadlocks.
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    $peakWorkingSet = [int64]0
    $memorySamples = 0
    do {
        try {
            $process.Refresh()
            if (-not $process.HasExited) {
                $samplePeak = [int64]$process.PeakWorkingSet64
                $sampleWorking = [int64]$process.WorkingSet64
                $peakWorkingSet = [math]::Max($peakWorkingSet, [math]::Max($samplePeak, $sampleWorking))
                $memorySamples++
            }
        } catch [System.InvalidOperationException] {
            # Process exited while counters were being read.
        }
    } while (-not $process.WaitForExit(25))
    $process.WaitForExit()
    $stdout = $stdoutTask.GetAwaiter().GetResult()
    $stderr = $stderrTask.GetAwaiter().GetResult()
    $wall.Stop()

    $exitCode = $process.ExitCode
    $cpuMs = $process.TotalProcessorTime.TotalMilliseconds
    $userCpuMs = $process.UserProcessorTime.TotalMilliseconds
    $kernelCpuMs = $process.PrivilegedProcessorTime.TotalMilliseconds

    $frames = @([regex]::Matches($stdout, '(?m)^frame=(\d+)\s*$') | ForEach-Object { [int64]$_.Groups[1].Value })
    $lastFrame = if ($frames.Count -gt 0) { $frames[-1] } else { 0 }
    $normalizedCpu = if ($wall.Elapsed.TotalMilliseconds -gt 0) {
        ($cpuMs / $wall.Elapsed.TotalMilliseconds) * 100.0 / [Environment]::ProcessorCount
    } else { 0.0 }

    [ordered]@{
        Backend = $Backend
        ExitCode = $exitCode
        WallMilliseconds = [math]::Round($wall.Elapsed.TotalMilliseconds, 3)
        CpuMilliseconds = [math]::Round($cpuMs, 3)
        UserCpuMilliseconds = [math]::Round($userCpuMs, 3)
        KernelCpuMilliseconds = [math]::Round($kernelCpuMs, 3)
        NormalizedCpuPercent = [math]::Round($normalizedCpu, 3)
        PeakWorkingSetMB = $(if ($memorySamples -gt 0 -and $peakWorkingSet -gt 0) { [math]::Round($peakWorkingSet / 1MB, 3) } else { $null })
        MemorySamples = $memorySamples
        Frames = $lastFrame
        FramesPerSecond = if ($wall.Elapsed.TotalSeconds -gt 0) { [math]::Round($lastFrame / $wall.Elapsed.TotalSeconds, 3) } else { 0.0 }
        Stderr = $stderr.Trim()
    }
}

Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_FFMPEG_ROOT)) 'SENTRIUM_FFMPEG_ROOT is not set'

$root = [IO.Path]::GetFullPath($env:SENTRIUM_FFMPEG_ROOT)
$ffmpeg = Resolve-SentriumExecutable $root 'ffmpeg.exe'
$version = (& $ffmpeg -version | Select-Object -First 1).Trim()
Assert-Sentrium ($version.StartsWith('ffmpeg version 9.0.2')) 'ffmpeg version must be 9.0.2'

$resolvedInput = Resolve-Path -LiteralPath $InputPath -ErrorAction SilentlyContinue
Assert-Sentrium ($null -ne $resolvedInput) "input sample not found: $InputPath"
$inputFilePath = $resolvedInput.ProviderPath

if ([string]::IsNullOrWhiteSpace($EvidenceDirectory)) {
    $EvidenceDirectory = Join-Path (Split-Path $inputFilePath -Parent) 'ffmpeg-benchmark'
}
if ([IO.Path]::IsPathRooted($EvidenceDirectory)) {
    $evidence = [IO.Path]::GetFullPath($EvidenceDirectory)
} else {
    $evidence = [IO.Path]::GetFullPath((Join-Path $PWD.ProviderPath $EvidenceDirectory))
}
[IO.Directory]::CreateDirectory($evidence) | Out-Null

$backends = @('Software','D3D11VA')
$results = @()

foreach ($backend in $backends) {
    $warmup = Invoke-SentriumDecode $backend $ffmpeg $inputFilePath $Loops
    Assert-Sentrium ($warmup.ExitCode -eq 0) "$backend warm-up failed: $($warmup.Stderr)"
    for ($i = 1; $i -le $Repetitions; $i++) {
        $run = Invoke-SentriumDecode $backend $ffmpeg $inputFilePath $Loops
        Assert-Sentrium ($run.ExitCode -eq 0) "$backend repetition $i failed: $($run.Stderr)"
        $run['Repetition'] = $i
        $results += [pscustomobject]$run
    }
}

$summary = @()
foreach ($backend in $backends) {
    $group = @($results | Where-Object { $_.Backend -eq $backend })
    $validMemoryRuns = @($group | Where-Object { $null -ne $_.PeakWorkingSetMB })
    $summary += [pscustomobject][ordered]@{
        Backend = $backend
        Repetitions = $group.Count
        FramesPerRun = [int64](($group | Measure-Object -Property Frames -Average).Average)
        MeanWallMilliseconds = [math]::Round(($group | Measure-Object -Property WallMilliseconds -Average).Average, 3)
        MeanCpuMilliseconds = [math]::Round(($group | Measure-Object -Property CpuMilliseconds -Average).Average, 3)
        MeanNormalizedCpuPercent = [math]::Round(($group | Measure-Object -Property NormalizedCpuPercent -Average).Average, 3)
        MeanPeakWorkingSetMB = $(if ($validMemoryRuns.Count -gt 0) { [math]::Round(($validMemoryRuns | Measure-Object -Property PeakWorkingSetMB -Average).Average, 3) } else { $null })
        MeanFramesPerSecond = [math]::Round(($group | Measure-Object -Property FramesPerSecond -Average).Average, 3)
    }
}

$output = [ordered]@{
    Benchmark = 'SENTRIUM-F004-FFMPEG-COMPARISON'
    InputFileName = [IO.Path]::GetFileName($inputFilePath)
    InputBytes = (Get-Item -LiteralPath $inputFilePath).Length
    FFmpeg = $version
    FFmpegSha256 = (Get-FileHash -LiteralPath $ffmpeg -Algorithm SHA256).Hash
    LoopCount = $Loops
    EffectivePasses = $Loops + 1
    Repetitions = $Repetitions
    LogicalProcessors = [Environment]::ProcessorCount
    Summary = $summary
    Runs = $results
    EvidenceDirectory = $evidence
}

$outputPath = Join-Path $evidence 'result.json'
$output | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $outputPath -Encoding UTF8
$output | ConvertTo-Json -Depth 8
