# Open the CMake coverage report in the default browser (PowerShell)
$REPORT = "index.html"
$REPORT_DIR = "coverage_report"
$REPORT_PATH = Join-Path $REPORT_DIR $REPORT
if (Test-Path $REPORT_PATH) {
    if ($IsLinux) {
        xdg-open $REPORT_PATH
    } elseif ($IsMacOS) {
        open $REPORT_PATH
    } elseif ($IsWindows) {
        Start-Process $REPORT_PATH
    } else {
        Write-Output "Unknown OS. Please open $REPORT_PATH manually."
    }
} else {
    Write-Output "Coverage report not found. Run gen_coverage.ps1 first."
}
