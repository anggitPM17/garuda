# Garuda Deterministic Build Verification Script
# Compiles project twice in separate directories and verifies SHA256 binary hash equality

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
Set-Location $ProjectDir

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "GARUDA DETERMINISTIC BUILD CHECKER" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host ""

$Det1 = "build_det1"
$Det2 = "build_det2"

# Clean previous test directories
if (Test-Path $Det1) { Remove-Item -Recurse -Force $Det1 }
if (Test-Path $Det2) { Remove-Item -Recurse -Force $Det2 }

$HasNinja = $null -ne (Get-Command ninja -ErrorAction SilentlyContinue)

try {
    # Build 1
    Write-Host "[1/2] Building Instance 1 ($Det1)..." -ForegroundColor Yellow
    if ($HasNinja) {
        cmake -B $Det1 -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
    } else {
        cmake -B $Det1 -DCMAKE_BUILD_TYPE=Release
    }
    cmake --build $Det1 --config Release --target garuda_tco_tests

    # Build 2
    Write-Host "[2/2] Building Instance 2 ($Det2)..." -ForegroundColor Yellow
    if ($HasNinja) {
        cmake -B $Det2 -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
    } else {
        cmake -B $Det2 -DCMAKE_BUILD_TYPE=Release
    }
    cmake --build $Det2 --config Release --target garuda_tco_tests

    # Resolve executable paths
    $PossiblePaths1 = @(
        "$Det1/garuda_tco_tests.exe",
        "$Det1/Release/garuda_tco_tests.exe",
        "$Det1/Debug/garuda_tco_tests.exe"
    )
    $PossiblePaths2 = @(
        "$Det2/garuda_tco_tests.exe",
        "$Det2/Release/garuda_tco_tests.exe",
        "$Det2/Debug/garuda_tco_tests.exe"
    )

    $Exe1 = $null
    foreach ($p in $PossiblePaths1) { if (Test-Path $p) { $Exe1 = $p; break } }

    $Exe2 = $null
    foreach ($p in $PossiblePaths2) { if (Test-Path $p) { $Exe2 = $p; break } }

    if ((-not $Exe1) -or (-not $Exe2)) {
        Write-Host "[FAIL] Executable not found in build directories!" -ForegroundColor Red
        exit 1
    }

    # Calculate SHA256 Hashes
    $Hash1 = (Get-FileHash $Exe1 -Algorithm SHA256).Hash
    $Hash2 = (Get-FileHash $Exe2 -Algorithm SHA256).Hash

    Write-Host ""
    Write-Host "Build 1 SHA256: $Hash1" -ForegroundColor Gray
    Write-Host "Build 2 SHA256: $Hash2" -ForegroundColor Gray
    Write-Host ""

    if ($Hash1 -eq $Hash2) {
        Write-Host "DETERMINISTIC: OK (Hashes match 100%)" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "NON-DETERMINISTIC: FAILED (Hashes differ)" -ForegroundColor Red
        exit 1
    }
}
finally {
    # Cleanup temporary build directories
    if (Test-Path $Det1) { Remove-Item -Recurse -Force $Det1 }
    if (Test-Path $Det2) { Remove-Item -Recurse -Force $Det2 }
}
