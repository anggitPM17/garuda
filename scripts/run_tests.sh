#!/usr/bin/env bash
# =====================================================================
# GARUDA BEAM — LINUX TEST RUNNER
# Menjalankan seluruh test suite Garuda BEAM di Linux/POSIX
# =====================================================================
set -e

BUILD_DIR="build"

echo "======================================================="
echo "  GARUDA BEAM — LINUX TEST SUITE RUNNER"
echo "======================================================="

TEST_EXECUTABLES=(
    "garuda_tests"
    "garuda_feature_tests"
    "garuda_lexer_tests"
    "garuda_parser_tests"
    "garuda_codegen_tests"
    "garuda_compiler_api_tests"
    "garuda_actor_tests"
    "garuda_bytecode_tests"
    "garuda_bytecode_io_tests"
    "garuda_tco_tests"
    "garuda_control_flow_tests"
    "garuda_isolation_tests"
    "garuda_fault_tests"
    "garuda_runtime_stress_tests"
    "garuda_memory_tests"
    "garuda_error_reporting_tests"
    "garuda_fuzzing_tests"
    "garuda_grdmix_e2e_tests"
    "garuda_http_server_e2e_tests"
    "garuda_websocket_e2e_tests"
    "garuda_richat_backend_tests"
    "garuda_protobuf_minimal_tests"
    "garuda_tls_tests"
    "garuda_sqlite_tests"
    "garuda_query_builder_tests"
    "garuda_schema_migrator_tests"
    "garuda_health_check_tests"
    "garuda_logger_tests"
    "garuda_metrics_tests"
    "garuda_tracing_tests"
    "garuda_jwt_tests"
    "garuda_security_app_tests"
    "garuda_secret_manager_tests"
    "garuda_c_abi_tests"
    "garuda_fs_process_tests"
    "garuda_doc_generator_tests"
    "garuda_cli_example_tests"
    "garuda_script_engine_tests"
    "garuda_actor_snapshot_tests"
    "garuda_nusantara_tests"
    "garuda_jalur_langit_tests"
    "garuda_connection_pool_tests"
    "garuda_query_cache_tests"
    "garuda_json_serializer_tests"
    "garuda_auth_manager_tests"
    "garuda_backup_manager_tests"
    "garuda_schema_migrator_jl_tests"
    "garuda_jalur_langit_bridge_tests"
    "garuda_autopilot_admin_gate_tests"
    "garuda_autopilot_health_monitor_tests"
    "garuda_autopilot_ai_code_fixer_tests"
    "garuda_autopilot_ai_tests"
    "garuda_richat_contract_tests"
    "garuda_richat_gateway_tests"
    "garuda_richat_room_tests"
    "garuda_richat_observability_tests"
    "garuda_richat_status_moment_tests"
    "garuda_richat_call_log_tests"
    "garuda_richat_server_smoke_tests"
    "garuda_gen_server_tests"
    "garuda_supervisor_tests"
    "garuda_application_tests"
    "garuda_pubsub_tests"
    "garuda_gen_statem_tests"
    "garuda_dist_node_tests"
    "garuda_global_registry_tests"
    "garuda_cluster_manager_tests"
    "garuda_pg_tests"
    "garuda_mnesia_tests"
    "garuda_mnesia_tier_tests"
    "garuda_mnesia_sweep_tests"
    "garuda_mnesia_cluster_tests"
    "garuda_crypto_tests"
    "garuda_key_transparency_tests"
    "garuda_sealed_sender_tests"
    "garuda_e2e_session_tests"
    "garuda_webrtc_signaling_tests"
    "garuda_presence_tests"
    "garuda_typing_indicator_tests"
    "garuda_read_receipt_tests"
    "garuda_reaction_tests"
    "garuda_shell_tests"
    "garuda_rpc_tests"
    "garuda_observer_tests"
    "garuda_telemetry_tests"
    "garuda_richat_app_tests"
    "garuda_richat_group_member_tests"
    "garuda_richat_group_fanout_tests"
    "garuda_richat_group_dist_tests"
    "garuda_richat_cold_db_tests"
)

PASSED=0
FAILED=0

for test_exe in "${TEST_EXECUTABLES[@]}"; do
    path="${BUILD_DIR}/${test_exe}"
    if [ -f "${path}" ]; then
        echo "Running ${test_exe}..."
        if "${path}"; then
            echo "  [PASS] ${test_exe}"
            PASSED=$((PASSED + 1))
        else
            echo "  [FAIL] ${test_exe}"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "  [SKIP] ${test_exe} (not found in ${BUILD_DIR}/)"
    fi
done

echo "======================================================="
echo "  SUMMARY: ${PASSED} Passed, ${FAILED} Failed"
echo "======================================================="

if [ "${FAILED}" -ne 0 ]; then
    exit 1
fi
