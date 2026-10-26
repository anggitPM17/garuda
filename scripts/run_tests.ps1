# Garuda Test Runner Script (Windows PowerShell)
# Builds and executes test suite with per-test timeout watchdog

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
Set-Location $ProjectDir

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "GARUDA LOCAL TEST RUNNER & WATCHDOG" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Configure CMake if needed
if (-not (Test-Path "build")) {
    Write-Host "[1/3] Configuring CMake..." -ForegroundColor Yellow
    cmake -B build -DCMAKE_BUILD_TYPE=Release
}

# Step 2: Build test targets
Write-Host "[2/3] Building Test Targets..." -ForegroundColor Yellow
cmake --build build --config Release --target garuda_tco_tests garuda_control_flow_tests garuda_isolation_tests garuda_fault_tests

if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] Build failed!" -ForegroundColor Red
    exit 1
}

# Step 3: Run tests with watchdog
Write-Host "`n[3/3] Running Test Suite with Watchdog..." -ForegroundColor Yellow
Write-Host ""

$Tests = @(
    @{ Name = "garuda_tco_tests"; Executable = "build/Release/garuda_tco_tests.exe" },
    @{ Name = "garuda_control_flow_tests"; Executable = "build/Release/garuda_control_flow_tests.exe" },
    @{ Name = "garuda_isolation_tests"; Executable = "build/Release/garuda_isolation_tests.exe" },
    @{ Name = "garuda_fault_tests"; Executable = "build/Release/garuda_fault_tests.exe" }
)

$AllPassed = $true

foreach ($Test in $Tests) {
    $ExePath = $Test.Executable
    $TestName = $Test.Name

    if (-not (Test-Path $ExePath)) {
        Write-Host "  [FAIL] $TestName : Executable not found ($ExePath)" -ForegroundColor Red
        $AllPassed = $false
        continue
    }

    $Proc = Start-Process -FilePath $ExePath -NoNewWindow -PassThru
    $Finished = $Proc | Wait-Process -Timeout 30 -ErrorAction SilentlyContinue

    if (-not $Proc.HasExited) {
        Stop-Process -Id $Proc.Id -Force
        Write-Host "  [FAIL] $TestName : TIMEOUT (>30s)" -ForegroundColor Red
        $AllPassed = $false
    } else {
        if ($Proc.ExitCode -eq 0) {
            Write-Host "  [PASS] $TestName : PASS (Exit 0)" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] $TestName : FAILED (Exit Code $($Proc.ExitCode))" -ForegroundColor Red
            $AllPassed = $false
        }
    }
}

Write-Host ""
if ($AllPassed) {
    Write-Host "[PASS] ALL TEST SUITES PASSED PERFECTLY!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "[FAIL] SOME TEST SUITES FAILED!" -ForegroundColor Red
    exit 1
}
