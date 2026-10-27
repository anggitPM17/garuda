# Garuda Performance Benchmark & Regression Prevention Script
# Runs garuda_bench_http_server.exe, parses req/sec throughput, and checks for regressions > 20% vs baseline (16,300 req/s)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
Set-Location $ProjectDir

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "GARUDA PERFORMANCE BENCHMARK SUITE" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host ""

# Ensure benchmark binary is built
if ((-not (Test-Path "build/garuda_bench_http_server.exe")) -and (-not (Test-Path "build/Release/garuda_bench_http_server.exe"))) {
    Write-Host "Building garuda_bench_http_server target..." -ForegroundColor Yellow
    cmake --build build --config Release --target garuda_bench_http_server
}

$BaselineReqSec = 16300.0
$Tolerance = 0.20 # Max 20% allowed performance drop
$MinAllowedReqSec = $BaselineReqSec * (1.0 - $Tolerance) # 13,040 req/s

$ExeName = "garuda_bench_http_server.exe"
$PossiblePaths = @(
    "build/$ExeName",
    "build/Release/$ExeName",
    "build/Debug/$ExeName"
)

$ExePath = $null
foreach ($p in $PossiblePaths) {
    if (Test-Path $p) {
        $ExePath = $p
        break
    }
}

if (-not $ExePath) {
    Write-Host "❌ Benchmark executable not found ($ExeName)!" -ForegroundColor Red
    exit 1
}

Write-Host "Baseline HTTP Throughput : $BaselineReqSec req/sec" -ForegroundColor Yellow
Write-Host "Minimum Acceptable Threshold (-20%) : $MinAllowedReqSec req/sec`n" -ForegroundColor Yellow

$RunThroughputs = @()

for ($run = 1; $run -le 2; $run++) {
    Write-Host "[RUN ${run}/2] Executing HTTP Server Benchmark..." -ForegroundColor Yellow

    $Output = & $ExePath 2>&1

    # Parse req/sec lines from benchmark output
    $Match = $Output | Select-String -Pattern '\((\d+(\.\d+)?) req/sec\)'

    if ($Match) {
        $LastMatch = $Match[-1].Matches.Groups[1].Value
        $ReqSec = [double]$LastMatch
        $RunThroughputs += $ReqSec
        Write-Host "  Result Run ${run}: $ReqSec req/sec" -ForegroundColor Green
    } else {
        Write-Host "❌ Failed to parse throughput output from benchmark!" -ForegroundColor Red
        Write-Host "Raw Output:`n$Output"
        exit 1
    }
}

# Calculate average throughput
$AvgReqSec = ($RunThroughputs | Measure-Object -Average).Average
Write-Host "`n=======================================================" -ForegroundColor Cyan
Write-Host ("Average Measured Throughput : {0:N2} req/sec" -f $AvgReqSec) -ForegroundColor Cyan
Write-Host ("Baseline Reference          : {0:N2} req/sec" -f $BaselineReqSec) -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan

if ($AvgReqSec -ge $MinAllowedReqSec) {
    Write-Host "`n✅ PERFORMANCE BENCHMARK: PASS (No regression detected)" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n❌ REGRESI PERFORMA DETECTED! (Throughput {0:N2} req/sec < Threshold {1:N2} req/sec)" -f $AvgReqSec, $MinAllowedReqSec -ForegroundColor Red
    exit 1
}
