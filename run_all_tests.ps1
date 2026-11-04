$passed = 0
$failed = 0
$timeout = 0
$failedList = @()
$timeoutList = @()

Get-ChildItem build\Release\*tests.exe | ForEach-Object {
    $name = $_.Name
    $pinfo = New-Object System.Diagnostics.ProcessStartInfo
    $pinfo.FileName = $_.FullName
    $pinfo.UseShellExecute = $false
    $pinfo.CreateNoWindow = $true

    $proc = [System.Diagnostics.Process]::Start($pinfo)
    $finished = $proc.WaitForExit(30000)

    if (-not $finished) {
        try { $proc.Kill() } catch {}
        $timeout++
        $timeoutList += $name
    } else {
        if ($proc.ExitCode -eq 0) {
            $passed++
        } else {
            $failed++
            $failedList += "$name (ExitCode: $($proc.ExitCode))"
        }
    }
    $proc.Close()
}

Write-Host "PASSED: $passed"
Write-Host "FAILED: $failed"
Write-Host "TIMEOUT: $timeout"
if ($failed -gt 0) {
    Write-Host "Failed executables:"
    $failedList | ForEach-Object { Write-Host "  - $_" }
}
if ($timeout -gt 0) {
    Write-Host "Timeout executables:"
    $timeoutList | ForEach-Object { Write-Host "  - $_" }
}
