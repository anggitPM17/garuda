# Garuda Performance Benchmark & Regression Prevention Script
# Verifies HTTP Server throughput (Baseline 16,300 req/s) and Actor Spawn throughput (Baseline 638,849 processes/s)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
Set-Location $ProjectDir

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "GARUDA PERFORMANCE BENCHMARK SUITE" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host ""

# 1. Build benchmark targets if needed
$HttpExeName = "garuda_bench_http_server.exe"
$SpawnExeName = "garuda_bench_spawn.exe"

if ((-not (Test-Path "build/$HttpExeName")) -and (-not (Test-Path "build/Release/$HttpExeName"))) {
    Write-Host "Building garuda_bench_http_server target..." -ForegroundColor Yellow
    cmake --build build --config Release --target garuda_bench_http_server
}

if ((-not (Test-Path "build/$SpawnExeName")) -and (-not (Test-Path "build/Release/$SpawnExeName"))) {
    Write-Host "Building garuda_bench_spawn target..." -ForegroundColor Yellow
    cmake --build build --config Release --target garuda_bench_spawn
}

# Resolve executable paths (Prefer Release over Debug)
$HttpExePath = $null
foreach ($p in @("build/Release/$HttpExeName", "build/$HttpExeName", "build/Debug/$HttpExeName")) {
    if (Test-Path $p) { $HttpExePath = $p; break }
}

$SpawnExePath = $null
foreach ($p in @("build/Release/$SpawnExeName", "build/$SpawnExeName", "build/Debug/$SpawnExeName")) {
    if (Test-Path $p) { $SpawnExePath = $p; break }
}

if (-not $HttpExePath) {
    Write-Host "❌ HTTP Benchmark executable not found ($HttpExeName)!" -ForegroundColor Red
    exit 1
}

if (-not $SpawnExePath) {
    Write-Host "❌ Spawn Benchmark executable not found ($SpawnExeName)!" -ForegroundColor Red
    exit 1
}

$RegressionDetected = $false

# --- Part A: HTTP Server Benchmark ---
Write-Host "-------------------------------------------------------" -ForegroundColor Yellow
Write-Host "[BENCH 1/2] HTTP SERVER THROUGHPUT BENCHMARK" -ForegroundColor Yellow
Write-Host "-------------------------------------------------------" -ForegroundColor Yellow

$HttpBaseline = 16300.0
$HttpMinAllowed = $HttpBaseline * 0.80 # 13,040 req/s

Write-Host "Baseline HTTP Throughput : $HttpBaseline req/sec" -ForegroundColor Yellow
Write-Host "Minimum Acceptable Threshold (-20%) : $HttpMinAllowed req/sec`n" -ForegroundColor Yellow

$HttpRuns = @()
for ($run = 1; $run -le 2; $run++) {
    Write-Host "[RUN ${run}/2] Executing HTTP Server Benchmark..." -ForegroundColor Yellow
    $Output = & $HttpExePath 2>&1
    $Match = $Output | Select-String -Pattern '\((\d+(\.\d+)?) req/sec\)'

    if ($Match) {
        $ReqSec = [double]$Match[-1].Matches.Groups[1].Value
        $HttpRuns += $ReqSec
        Write-Host "  Result Run ${run}: $ReqSec req/sec" -ForegroundColor Green
    } else {
        Write-Host "❌ Failed to parse HTTP throughput output!" -ForegroundColor Red
        exit 1
    }
}

$HttpAvg = ($HttpRuns | Measure-Object -Average).Average
Write-Host ("`nAverage HTTP Throughput : {0:N2} req/sec" -f $HttpAvg) -ForegroundColor Cyan

if ($HttpAvg -ge $HttpMinAllowed) {
    Write-Host "✅ HTTP SERVER BENCHMARK: PASS" -ForegroundColor Green
} else {
    $ErrMsg = "❌ REGRESI PERFORMA HTTP DETECTED! ({0:N2} < {1:N2})" -f $HttpAvg, $HttpMinAllowed
    Write-Host $ErrMsg -ForegroundColor Red
    $RegressionDetected = $true
}

# --- Part B: Actor Spawn Throughput Benchmark ---
Write-Host "`n-------------------------------------------------------" -ForegroundColor Yellow
Write-Host "[BENCH 2/2] ACTOR SPAWN THROUGHPUT BENCHMARK" -ForegroundColor Yellow
Write-Host "-------------------------------------------------------" -ForegroundColor Yellow

$SpawnBaseline = 638849.0
$SpawnMinAllowed = 400000.0 # Threshold for local dev & CI hardware variance

Write-Host "Baseline Spawn Throughput : $SpawnBaseline processes/sec" -ForegroundColor Yellow
Write-Host "Minimum Acceptable Threshold : $SpawnMinAllowed processes/sec`n" -ForegroundColor Yellow

$SpawnRuns = @()
for ($run = 1; $run -le 2; $run++) {
    Write-Host "[RUN ${run}/2] Executing Spawn Throughput Benchmark..." -ForegroundColor Yellow
    $Output = & $SpawnExePath 2>&1
    $Match = $Output | Select-String -Pattern '(\d+(\.\d+)?) processes/sec'

    if ($Match) {
        $ProcSec = [double]$Match[-1].Matches.Groups[1].Value
        $SpawnRuns += $ProcSec
        $Msg = "  Result Run {0}: {1:N2} processes/sec" -f $run, $ProcSec
        Write-Host $Msg -ForegroundColor Green
    } else {
        Write-Host "❌ Failed to parse Spawn throughput output!" -ForegroundColor Red
        exit 1
    }
}

$SpawnAvg = ($SpawnRuns | Measure-Object -Average).Average
Write-Host ("`nAverage Spawn Throughput : {0:N2} processes/sec" -f $SpawnAvg) -ForegroundColor Cyan

if ($SpawnAvg -ge $SpawnMinAllowed) {
    Write-Host "✅ ACTOR SPAWN BENCHMARK: PASS" -ForegroundColor Green
} else {
    $ErrMsg = "❌ REGRESI SPAWN DETECTED! ({0:N2} < {1:N2})" -f $SpawnAvg, $SpawnMinAllowed
    Write-Host $ErrMsg -ForegroundColor Red
    $RegressionDetected = $true
}

Write-Host "`n=======================================================" -ForegroundColor Cyan
if ($RegressionDetected) {
    Write-Host "❌ REGRESI PERFORMA DETECTED IN BENCHMARK SUITE!" -ForegroundColor Red
    exit 1
} else {
    Write-Host "🎉 ALL PERFORMANCE BENCHMARKS PASSED PERFECTLY!" -ForegroundColor Green
    exit 0
}
