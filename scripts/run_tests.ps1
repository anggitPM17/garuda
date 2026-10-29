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
cmake --build build --config Release --target garuda_tco_tests garuda_control_flow_tests garuda_isolation_tests garuda_fault_tests garuda_runtime_stress_tests garuda_memory_tests garuda_error_reporting_tests garuda_fuzzing_tests garuda_grdmix_e2e_tests garuda_bytecode_io_tests garuda_compiler_api_tests garuda_formatter_linter_tests garuda_http_server_e2e_tests garuda_websocket_e2e_tests garuda_richat_backend_tests garuda_protobuf_minimal_tests garuda_tls_tests garuda_sqlite_tests garuda_query_builder_tests garuda_schema_migrator_tests garuda_health_check_tests garuda_logger_tests garuda_metrics_tests garuda_tracing_tests garuda_jwt_tests garuda_security_app_tests garuda_secret_manager_tests garuda_c_abi_tests garuda_fs_process_tests garuda_doc_generator_tests garuda_cli_example_tests garuda_script_engine_tests garuda_actor_snapshot_tests garuda_nusantara_tests garuda_jalur_langit_tests garuda_connection_pool_tests garuda_query_cache_tests garuda_json_serializer_tests garuda_auth_manager_tests garuda_backup_manager_tests garuda_schema_migrator_jl_tests garuda_jalur_langit_bridge_tests garuda_autopilot_admin_gate_tests garuda_autopilot_health_monitor_tests garuda_autopilot_ai_code_fixer_tests garuda_autopilot_ai_tests garuda_richat_contract_tests garuda_richat_gateway_tests garuda_richat_room_tests garuda_richat_observability_tests garuda_richat_status_moment_tests garuda_richat_call_log_tests garuda_richat_server garuda_richat_server_smoke_tests garuda_gen_server_tests garuda_supervisor_tests garuda_application_tests garuda_pubsub_tests garuda_gen_statem_tests garuda_dist_node_tests garuda_global_registry_tests garuda_cluster_manager_tests garuda_pg_tests garuda_mnesia_tests garuda_mnesia_tier_tests garuda_mnesia_sweep_tests garuda_mnesia_cluster_tests garuda_crypto_tests garuda_key_transparency_tests garuda_sealed_sender_tests garuda_e2e_session_tests garuda_webrtc_signaling_tests garuda_presence_tests garuda_typing_indicator_tests garuda_read_receipt_tests garuda_reaction_tests garuda_shell_tests garuda_rpc_tests garuda_observer_tests garuda_telemetry_tests garuda_richat_app_tests garuda_richat_group_member_tests garuda_richat_group_fanout_tests garuda_richat_group_dist_tests garuda_richat_cold_db_tests garuda_anomaly_detector_tests garuda_rca_engine_tests garuda_action_engine_tests garuda_autopilot_runtime_tests garuda_native_websocket_tests garuda_native_crypto_tests garuda_native_sync_tests garuda_jni_bridge_tests garuda_rate_limiter_tests garuda_totp_2fa_tests garuda_captcha_pow_tests garuda_audit_log_tests garuda_ffi_tests garuda_ffi_registry_tests garuda_native_binding_tests garuda_dsl_tests garuda_richat_chat_api_tests garuda_richat_group_api_tests garuda_richat_realtime_tests garuda_richat_offline_queue_tests

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
  "garuda_jalur_langit_tests",
  "garuda_connection_pool_tests",
  "garuda_query_cache_tests",
  "garuda_json_serializer_tests",
  "garuda_auth_manager_tests",
  "garuda_backup_manager_tests",
  "garuda_schema_migrator_jl_tests",
  "garuda_jalur_langit_bridge_tests",
  "garuda_autopilot_admin_gate_tests",
  "garuda_autopilot_health_monitor_tests",
  "garuda_autopilot_ai_code_fixer_tests",
  "garuda_autopilot_ai_tests",
  "garuda_richat_contract_tests",
  "garuda_richat_gateway_tests",
  "garuda_richat_room_tests",
  "garuda_richat_observability_tests",
  "garuda_richat_status_moment_tests",
  "garuda_richat_call_log_tests",
  "garuda_richat_server_smoke_tests",
  "garuda_gen_server_tests",
  "garuda_supervisor_tests",
  "garuda_application_tests",
  "garuda_pubsub_tests",
  "garuda_gen_statem_tests",
  "garuda_dist_node_tests",
  "garuda_global_registry_tests",
  "garuda_cluster_manager_tests",
  "garuda_pg_tests",
  "garuda_mnesia_tests",
  "garuda_mnesia_tier_tests",
  "garuda_mnesia_sweep_tests",
  "garuda_mnesia_cluster_tests",
  "garuda_crypto_tests",
  "garuda_key_transparency_tests",
  "garuda_sealed_sender_tests",
  "garuda_e2e_session_tests",
  "garuda_webrtc_signaling_tests",
  "garuda_presence_tests",
  "garuda_typing_indicator_tests",
  "garuda_read_receipt_tests",
  "garuda_reaction_tests",
  "garuda_shell_tests",
  "garuda_rpc_tests",
  "garuda_observer_tests",
  "garuda_telemetry_tests",
  "garuda_richat_app_tests",
  "garuda_richat_group_member_tests",
  "garuda_richat_group_fanout_tests",
  "garuda_richat_group_dist_tests",
  "garuda_richat_cold_db_tests",
  "garuda_anomaly_detector_tests",
  "garuda_rca_engine_tests",
  "garuda_action_engine_tests",
  "garuda_autopilot_runtime_tests",
  "garuda_native_websocket_tests",
  "garuda_native_crypto_tests",
  "garuda_native_sync_tests",
  "garuda_jni_bridge_tests",
  "garuda_rate_limiter_tests",
  "garuda_totp_2fa_tests",
  "garuda_captcha_pow_tests",
  "garuda_audit_log_tests",
  "garuda_ffi_tests",
  "garuda_ffi_registry_tests",
  "garuda_native_binding_tests",
  "garuda_dsl_tests",
  "garuda_richat_chat_api_tests",
  "garuda_richat_group_api_tests",
  "garuda_richat_realtime_tests",
  "garuda_richat_offline_queue_tests"
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
