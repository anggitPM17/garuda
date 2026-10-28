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
    cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
}

# Step 2: Build test targets
Write-Host "[2/3] Building Test Targets..." -ForegroundColor Yellow
cmake --build build --config Release --target garuda_tco_tests garuda_control_flow_tests garuda_isolation_tests garuda_fault_tests garuda_runtime_stress_tests garuda_memory_tests garuda_error_reporting_tests garuda_fuzzing_tests garuda_grdmix_e2e_tests garuda_bytecode_io_tests garuda_compiler_api_tests garuda_formatter_linter_tests garuda_http_server_e2e_tests garuda_websocket_e2e_tests garuda_richat_backend_tests garuda_protobuf_minimal_tests garuda_tls_tests garuda_sqlite_tests garuda_query_builder_tests garuda_schema_migrator_tests garuda_health_check_tests garuda_logger_tests garuda_metrics_tests garuda_tracing_tests garuda_jwt_tests garuda_security_app_tests garuda_secret_manager_tests garuda_c_abi_tests garuda_fs_process_tests garuda_doc_generator_tests garuda_cli_example_tests garuda_script_engine_tests garuda_actor_snapshot_tests garuda_nusantara_tests garuda_jalur_langit_tests

if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] Build failed!" -ForegroundColor Red
    exit 1
}

# Step 3: Run tests with watchdog
Write-Host "`n[3/3] Running Test Suite with Watchdog..." -ForegroundColor Yellow
Write-Host ""

$TestNames = @(
    "garuda_tco_tests",
    "garuda_control_flow_tests",
    "garuda_isolation_tests",
    "garuda_fault_tests",
    "garuda_runtime_stress_tests",
    "garuda_memory_tests",
    "garuda_error_reporting_tests",
    "garuda_fuzzing_tests",
    "garuda_grdmix_e2e_tests",
    "garuda_bytecode_io_tests",
    "garuda_compiler_api_tests",
    "garuda_formatter_linter_tests",
    "garuda_http_server_e2e_tests",
    "garuda_websocket_e2e_tests",
    "garuda_richat_backend_tests",
    "garuda_protobuf_minimal_tests",
    "garuda_tls_tests",
    "garuda_sqlite_tests",
    "garuda_query_builder_tests",
    "garuda_schema_migrator_tests",
    "garuda_health_check_tests",
    "garuda_logger_tests",
    "garuda_metrics_tests",
    "garuda_tracing_tests",
    "garuda_jwt_tests",
    "garuda_security_app_tests",
    "garuda_secret_manager_tests",
    "garuda_c_abi_tests",
    "garuda_fs_process_tests",
    "garuda_doc_generator_tests",
    "garuda_cli_example_tests",
    "garuda_script_engine_tests",
  "garuda_actor_snapshot_tests",
  "garuda_nusantara_tests",
  "garuda_jalur_langit_tests"
)

$AllPassed = $true

foreach ($TestName in $TestNames) {
    $ExeName = "$TestName.exe"
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
        Write-Host "  [FAIL] $TestName : Executable not found ($ExeName)" -ForegroundColor Red
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
