param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [ValidateRange(1, 50)][int]$Cycles = 10
)

$ErrorActionPreference = 'Stop'

function Assert-Sentrium([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "CLIENT SMOKE FAIL: $Message" }
}

Assert-Sentrium (Test-Path -LiteralPath $Executable) "Executable not found: $Executable"
Assert-Sentrium (-not [string]::IsNullOrWhiteSpace($env:SENTRIUM_QT_ROOT)) 'SENTRIUM_QT_ROOT missing'

$qtBin = Join-Path $env:SENTRIUM_QT_ROOT 'bin'
Assert-Sentrium (Test-Path -LiteralPath (Join-Path $qtBin 'Qt6Core.dll')) 'Qt runtime not found'
$env:PATH = "$qtBin;$env:PATH"

$existing = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
Assert-Sentrium ($existing.Count -eq 0) 'sentrium-client is already running'

if (-not ('SentriumNative.Window' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

namespace SentriumNative {
    public static class Window {
        [StructLayout(LayoutKind.Sequential)]
        public struct RECT {
            public int Left;
            public int Top;
            public int Right;
            public int Bottom;
        }

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool MoveWindow(
            IntPtr hWnd, int x, int y, int width, int height, bool repaint);

        [DllImport("user32.dll")]
        public static extern uint GetDpiForWindow(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool PostMessage(
            IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
    }
}
'@
}

$logPath = Join-Path $env:LOCALAPPDATA 'Sentrium\VMS\dev\Foundation-003\client-runtime\logs\sentrium-0.jsonl'
$WM_CLOSE = 0x0010

for ($cycle = 1; $cycle -le $Cycles; ++$cycle) {
    $process = $null
    try {
        $process = Start-Process -FilePath $Executable -PassThru

        try {
            [void]$process.WaitForInputIdle(5000)
        } catch {
            # MainWindowHandle polling below remains the authoritative check.
        }

        $deadline = [DateTime]::UtcNow.AddSeconds(8)
        $hwnd = [IntPtr]::Zero
        while ([DateTime]::UtcNow -lt $deadline) {
            if ($process.HasExited) { break }
            $process.Refresh()
            $hwnd = $process.MainWindowHandle
            if ($hwnd -ne [IntPtr]::Zero) { break }
            Start-Sleep -Milliseconds 100
        }

        Assert-Sentrium (-not $process.HasExited) "cycle $cycle exited before window creation"
        Assert-Sentrium ($hwnd -ne [IntPtr]::Zero) "cycle $cycle main window not found"
        Assert-Sentrium ($process.MainWindowTitle -like 'SENTRIUM VMS*') "cycle $cycle unexpected window title"

        $dpi = [SentriumNative.Window]::GetDpiForWindow($hwnd)
        Assert-Sentrium ($dpi -gt 0) "cycle $cycle DPI query failed"

        # QML minimumWidth/minimumHeight apply to the client area. On Windows,
        # the non-client frame/title bar makes the minimum outer size larger.
        # Use a target safely above that minimum so this check validates resize
        # behavior instead of testing decoration metrics.
        $targetWidth = 1200
        $targetHeight = 800
        $moved = [SentriumNative.Window]::MoveWindow(
            $hwnd, 80, 80, $targetWidth, $targetHeight, $true)
        Assert-Sentrium $moved "cycle $cycle resize failed"
        Start-Sleep -Milliseconds 150

        $rect = New-Object SentriumNative.Window+RECT
        $rectOk = [SentriumNative.Window]::GetWindowRect($hwnd, [ref]$rect)
        Assert-Sentrium $rectOk "cycle $cycle window bounds unavailable"
        $width = $rect.Right - $rect.Left
        $height = $rect.Bottom - $rect.Top
        Assert-Sentrium (
            [Math]::Abs($width - $targetWidth) -le 2 -and
            [Math]::Abs($height - $targetHeight) -le 2
        ) "cycle $cycle resize not applied: $($width)x$($height)"

        $closePosted = [SentriumNative.Window]::PostMessage(
            $hwnd, $WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero)
        Assert-Sentrium $closePosted "cycle $cycle WM_CLOSE failed"
        Assert-Sentrium ($process.WaitForExit(8000)) "cycle $cycle shutdown timeout"
        Assert-Sentrium ($process.ExitCode -eq 0) "cycle $cycle exit code $($process.ExitCode)"

        Assert-Sentrium (Test-Path -LiteralPath $logPath) "cycle $cycle runtime log missing"
        $log = Get-Content -LiteralPath $logPath -Raw
        foreach ($event in @(
            '"EventId":"bootstrap-started"',
            '"EventId":"ui-initializing"',
            '"EventId":"window-presented"',
            '"EventId":"shutdown-started"',
            '"EventId":"shutdown-completed"'
        )) {
            Assert-Sentrium ($log.Contains($event)) "cycle $cycle missing log event $event"
        }

        Start-Sleep -Milliseconds 100
        $residual = @(Get-Process -Name 'sentrium-client' -ErrorAction SilentlyContinue)
        Assert-Sentrium ($residual.Count -eq 0) "cycle $cycle residual process detected"

        Write-Host ("PASS F002 client cycle {0}/{1}; DPI={2}; size={3}x{4}" -f $cycle, $Cycles, $dpi, $width, $height)
    } finally {
        if ($null -ne $process -and -not $process.HasExited) {
            $process.Kill()
            [void]$process.WaitForExit(5000)
        }
        if ($null -ne $process) { $process.Dispose() }
    }
}

Write-Host "PASS F002 client smoke: $Cycles clean open/resize/DPI/close cycles"
