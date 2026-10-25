param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [ValidateRange(10, 300)][int]$IdleSeconds = 60
)

$ErrorActionPreference = 'Stop'

function Assert-Sentrium([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "CLIENT OBSERVATION FAIL: $Message" }
}

Assert-Sentrium (Test-Path -LiteralPath $Executable) "Executable not found: $Executable"
Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_QT_ROOT)) 'SENTRIUM_QT_ROOT missing'

$qtBin = Join-Path $env:SENTRIUM_QT_ROOT 'bin'
Assert-Sentrium (Test-Path -LiteralPath (Join-Path $qtBin 'Qt6Core.dll')) 'Qt runtime not found'
$env:PATH = "$qtBin;$env:PATH"

$existing = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
Assert-Sentrium ($existing.Count -eq 0) 'sentrium-client is already running'

$wall = [Diagnostics.Stopwatch]::StartNew()
$process = $null

try {
    $process = Start-Process -FilePath $Executable -PassThru
    $startupDeadline = [DateTime]::UtcNow.AddSeconds(10)
    $hwnd = [IntPtr]::Zero

    while ([DateTime]::UtcNow -lt $startupDeadline) {
        if ($process.HasExited) { break }
        $process.Refresh()
        $hwnd = $process.MainWindowHandle
        if ($hwnd -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 50
    }

    Assert-Sentrium (-not $process.HasExited) 'process exited before window creation'
    Assert-Sentrium ($hwnd -ne [IntPtr]::Zero) 'main window not found'

    $startupMs = $wall.Elapsed.TotalMilliseconds
    $process.Refresh()
    $cpuStart = $process.TotalProcessorTime.TotalMilliseconds
    $workingSetStart = $process.WorkingSet64
    $privateStart = $process.PrivateMemorySize64
    $peakStart = $process.PeakWorkingSet64

    $samples = New-Object System.Collections.Generic.List[object]
    for ($second = 1; $second -le $IdleSeconds; ++$second) {
        Start-Sleep -Seconds 1
        Assert-Sentrium (-not $process.HasExited) "process exited during idle observation at second $second"
        $process.Refresh()
        $samples.Add([ordered]@{
            second = $second
            cpuTotalMs = [Math]::Round($process.TotalProcessorTime.TotalMilliseconds, 3)
            workingSetBytes = $process.WorkingSet64
            privateBytes = $process.PrivateMemorySize64
            peakWorkingSetBytes = $process.PeakWorkingSet64
            handleCount = $process.HandleCount
            threadCount = $process.Threads.Count
        })
    }

    $process.Refresh()
    $cpuEnd = $process.TotalProcessorTime.TotalMilliseconds
    $workingSetEnd = $process.WorkingSet64
    $privateEnd = $process.PrivateMemorySize64
    $peakEnd = $process.PeakWorkingSet64

    $shutdown = [Diagnostics.Stopwatch]::StartNew()
    Assert-Sentrium ($process.CloseMainWindow()) 'CloseMainWindow failed'
    Assert-Sentrium ($process.WaitForExit(10000)) 'shutdown timeout'
    $shutdown.Stop()
    Assert-Sentrium ($process.ExitCode -eq 0) "exit code $($process.ExitCode)"

    Start-Sleep -Milliseconds 200
    $residual = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
    Assert-Sentrium ($residual.Count -eq 0) 'residual process detected'

    $cpuDeltaMs = $cpuEnd - $cpuStart
    $logicalProcessors = [Environment]::ProcessorCount
    $averageCpuPercent = if ($IdleSeconds -gt 0 -and $logicalProcessors -gt 0) {
        ($cpuDeltaMs / ($IdleSeconds * 1000.0 * $logicalProcessors)) * 100.0
    } else { 0.0 }

    $result = [ordered]@{
        result = 'OBSERVATION COMPLETE'
        note = 'Observational only; no performance threshold is applied.'
        executable = [IO.Path]::GetFullPath($Executable)
        idleSeconds = $IdleSeconds
        startupToWindowMs = [Math]::Round($startupMs, 3)
        shutdownMs = [Math]::Round($shutdown.Elapsed.TotalMilliseconds, 3)
        exitCode = $process.ExitCode
        logicalProcessors = $logicalProcessors
        idleCpuDeltaMs = [Math]::Round($cpuDeltaMs, 3)
        idleAverageCpuPercentNormalized = [Math]::Round($averageCpuPercent, 4)
        workingSetStartBytes = $workingSetStart
        workingSetEndBytes = $workingSetEnd
        workingSetDeltaBytes = $workingSetEnd - $workingSetStart
        privateStartBytes = $privateStart
        privateEndBytes = $privateEnd
        privateDeltaBytes = $privateEnd - $privateStart
        peakWorkingSetStartBytes = $peakStart
        peakWorkingSetEndBytes = $peakEnd
        residualProcessCount = $residual.Count
        samples = $samples
    }

    $json = $result | ConvertTo-Json -Depth 6
    Write-Output $json
} finally {
    if ($null -ne $process -and -not $process.HasExited) {
        $process.Kill()
        [void]$process.WaitForExit(5000)
    }
    if ($null -ne $process) { $process.Dispose() }
}
