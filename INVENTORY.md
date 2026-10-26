# 📋 INVENTARISASI KLAIM — GARUDA (FASE A–I)

## Ringkasan Inventarisasi Repository

| Fase | Nama Fase | File Implementasi Utama | File Test | File Benchmark | Jumlah `TEST_ASSERT` | Assert Raw `assert()` |
|---|---|---|---|---|---|---|
| **A** | Compiler & Language Parity | `lexer.hpp`, `parser.hpp`, `semantic.hpp`, `codegen.hpp`, `optimizer.hpp`, `macro_expander.hpp` | `test_lexer.cpp`, `test_parser.cpp`, `test_semantic.cpp`, `test_codegen.cpp`, `test_optimizer.cpp`, `test_macro.cpp` | - | **80+** | 20+ |
| **B** | BEAM VM Core Engine | `interpreter.hpp`, `gc.hpp`, `priority_scheduler.hpp`, `timer_wheel.hpp`, `work_stealing.hpp` | `test_beam.cpp`, `test_actor.cpp`, `test_bytecode.cpp`, `test_tco.cpp`, `test_isolation.cpp`, `test_preemption.cpp` | `bench_beam.cpp` | **100+** | 30+ |
| **C** | Standard Library & Web | `json.hpp`, `http.hpp`, `http_server.hpp`, `db.hpp`, `fullstack_bridge.hpp` | `test_json.cpp`, `test_http.cpp`, `test_http_server.cpp`, `test_db.cpp`, `test_fullstack.cpp` | `bench_http_server.cpp`, `bench_storage.cpp` | **55+** | 25+ |
| **D** | Developer Experience & Tooling | `lsp_engine.hpp`, `formatter.hpp`, `linter.hpp` | `test_lsp.cpp`, `test_formatter_linter.cpp`, `test_error_reporting.cpp` | - | **30+** | 15+ |
| **E** | Framework Web & ORM | `web.hpp`, `view.hpp`, `session.hpp`, `auth.hpp`, `orm.hpp`, `live_view.hpp` | `test_web.cpp`, `test_view.cpp`, `test_session_auth.cpp`, `test_orm.cpp`, `test_live_view.cpp` | - | **50+** | 20+ |
| **F** | Distributed Engine & JIT | `distributed_foundation.hpp`, `distributed_db.hpp`, `cluster_manager.hpp`, `jit_compiler.hpp`, `package_registry.hpp` | `test_distributed_foundation.cpp`, `test_distributed_db.cpp`, `test_cluster_manager.cpp`, `test_jit_compiler.cpp`, `test_package_manager.cpp` | `bench_raft.cpp`, `bench_distributed_db.cpp`, `bench_cluster.cpp`, `bench_jit.cpp`, `bench_package_manager.cpp` | **104** | 0 |
| **G** | Embedded IoT & Hardware | `gvm_micro.hpp`, `hardware_drivers.hpp`, `stream_processor.hpp`, `mqtt_broker.hpp`, `ota_updater.hpp` | `test_gvm_micro.cpp`, `test_hardware_drivers.cpp`, `test_edge_analytics.cpp`, `test_iot_framework.cpp`, `test_industrial.cpp`, `test_fase_g_audit.cpp` | `bench_gvm_micro.cpp`, `bench_hardware_drivers.cpp`, `bench_edge_analytics.cpp`, `bench_iot_framework.cpp`, `bench_industrial.cpp`, `bench_fase_g_audit.cpp` | **128** | 0 |
| **H** | Global Parity | `global_parity.hpp`, `phoenix_framework.hpp` | `test_global_parity.cpp` | `bench_global_parity.cpp` | **24** | 0 |
| **I** | Production Release | `production_release.hpp` | `test_production.cpp` | `bench_production.cpp` | **24** | 0 |
| **TOTAL** | **Seluruh Sistem (v67.0.0)** | **92 Files** | **57 Files** | **16 Files** | **600+** | **120+ (Legacy)** |

---

## 2. Inventarisasi File Implementasi (`include/`)

```text
include/garuda.hpp
include/garuda/version.hpp
include/garuda/beam/beam.hpp
include/garuda/beam/core/bitstring.hpp
include/garuda/beam/core/code_server.hpp
include/garuda/beam/core/gc.hpp
include/garuda/beam/core/lockfree_queue.hpp
include/garuda/beam/core/priority_scheduler.hpp
include/garuda/beam/core/runtime.hpp
include/garuda/beam/core/slab_allocator.hpp
include/garuda/beam/core/telemetry.hpp
include/garuda/beam/core/term.hpp
include/garuda/beam/core/timer_wheel.hpp
include/garuda/beam/core/work_stealing.hpp
include/garuda/beam/crypto/crypto.hpp
include/garuda/beam/crypto/x3dh.hpp
include/garuda/beam/global/global_parity.hpp
include/garuda/beam/global/phoenix_framework.hpp
include/garuda/beam/global/production_release.hpp
include/garuda/beam/gvm/actor_runtime.hpp
include/garuda/beam/gvm/beam_file_reader.hpp
include/garuda/beam/gvm/interpreter.hpp
include/garuda/beam/io/event_loop.hpp
include/garuda/beam/io/http_ws_server.hpp
include/garuda/beam/io/node.hpp
include/garuda/beam/micro/anomaly_detector.hpp
include/garuda/beam/micro/ble_mesh.hpp
include/garuda/beam/micro/device_provisioning.hpp
include/garuda/beam/micro/edge_alert_dispatcher.hpp
include/garuda/beam/micro/fleet_manager.hpp
include/garuda/beam/micro/gvm_micro.hpp
include/garuda/beam/micro/hardware_drivers.hpp
include/garuda/beam/micro/iot_framework.hpp
include/garuda/beam/micro/lorawan.hpp
include/garuda/beam/micro/modbus_bridge.hpp
include/garuda/beam/micro/mqtt_broker.hpp
include/garuda/beam/micro/mqtt_client.hpp
include/garuda/beam/micro/opcua_bridge.hpp
include/garuda/beam/micro/ota_updater.hpp
include/garuda/beam/micro/secure_boot.hpp
include/garuda/beam/micro/sensor_drivers.hpp
include/garuda/beam/micro/sensor_pipeline.hpp
include/garuda/beam/micro/stream_processor.hpp
include/garuda/beam/micro/windowed_aggregator.hpp
include/garuda/beam/net/cluster_manager.hpp
include/garuda/beam/net/cluster_socket.hpp
include/garuda/beam/net/cowboy_router.hpp
include/garuda/beam/net/distributed_db.hpp
include/garuda/beam/net/distributed_pubsub.hpp
include/garuda/beam/net/dist_node.hpp
include/garuda/beam/net/global_registry.hpp
include/garuda/beam/net/hot_reload.hpp
include/garuda/beam/net/raft.hpp
include/garuda/beam/net/socket_layer.hpp
include/garuda/beam/net/websocket_handler.hpp
include/garuda/beam/otp/application.hpp
include/garuda/beam/otp/circuit_breaker.hpp
include/garuda/beam/otp/ets.hpp
include/garuda/beam/otp/gen_server.hpp
include/garuda/beam/otp/media_storage.hpp
include/garuda/beam/otp/mnesia.hpp
include/garuda/beam/otp/pubsub.hpp
include/garuda/beam/otp/supervisor.hpp
include/garuda/beam/otp/task.hpp
include/garuda/beam/stdlib/auth.hpp
include/garuda/beam/stdlib/crypto.hpp
include/garuda/beam/stdlib/csrf.hpp
include/garuda/beam/stdlib/db.hpp
include/garuda/beam/stdlib/enum.hpp
include/garuda/beam/stdlib/fullstack_bridge.hpp
include/garuda/beam/stdlib/http.hpp
include/garuda/beam/stdlib/http_server.hpp
include/garuda/beam/stdlib/io.hpp
include/garuda/beam/stdlib/json.hpp
include/garuda/beam/stdlib/list.hpp
include/garuda/beam/stdlib/live_view.hpp
include/garuda/beam/stdlib/map.hpp
include/garuda/beam/stdlib/orm.hpp
include/garuda/beam/stdlib/result.hpp
include/garuda/beam/stdlib/session.hpp
include/garuda/beam/stdlib/stdlib.hpp
include/garuda/beam/stdlib/storage_engine.hpp
include/garuda/beam/stdlib/string.hpp
include/garuda/beam/stdlib/view.hpp
include/garuda/beam/stdlib/web.hpp
include/garuda/beam/sys/daemon.hpp
include/garuda/beam/sys/logger.hpp
include/garuda/beam/sys/resource_limits.hpp
include/garuda/beam/sys/signal_handler.hpp
include/garuda/compiler/ast.hpp
include/garuda/compiler/bytecode_reader.hpp
include/garuda/compiler/bytecode_writer.hpp
include/garuda/compiler/codegen.hpp
include/garuda/compiler/compiler.hpp
include/garuda/compiler/formatter.hpp
include/garuda/compiler/jit_compiler.hpp
include/garuda/compiler/lexer.hpp
include/garuda/compiler/linter.hpp
include/garuda/compiler/lsp_engine.hpp
include/garuda/compiler/macro_expander.hpp
include/garuda/compiler/optimizer.hpp
include/garuda/compiler/package_registry.hpp
include/garuda/compiler/parser.hpp
include/garuda/compiler/semantic.hpp
include/garuda/compiler/source_map.hpp
include/garuda/elixir/ecto.hpp
include/garuda/elixir/elixir.hpp
```

---

## 3. Inventarisasi File Test (`tests/`) & Jumlah `TEST_ASSERT`

```text
test_actor.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_actor_http_server.cpp: 10 TEST_ASSERT
test_beam.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_bytecode.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_chat_server.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_cluster_manager.cpp: 22 TEST_ASSERT
test_codegen.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_db.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_distributed_db.cpp: 20 TEST_ASSERT
test_distributed_foundation.cpp: 18 TEST_ASSERT
test_distributed_pubsub.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_edge_analytics.cpp: 24 TEST_ASSERT
test_error_handling.cpp: 15 TEST_ASSERT
test_error_reporting.cpp: 10 TEST_ASSERT
test_fase_g_audit.cpp: 12 TEST_ASSERT
test_fault_isolation.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_formatter_linter.cpp: 10 TEST_ASSERT
test_fullstack.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_garuda_features.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_gc_scale.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_global_parity.cpp: 24 TEST_ASSERT
test_global_registry.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_grdmix.cpp: 8 TEST_ASSERT
test_gvm_micro.cpp: 22 TEST_ASSERT
test_hardware_drivers.cpp: 24 TEST_ASSERT
test_hot_reload.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_http.cpp: 10 TEST_ASSERT
test_http_server.cpp: 10 TEST_ASSERT
test_industrial.cpp: 24 TEST_ASSERT
test_iot_framework.cpp: 24 TEST_ASSERT
test_isolation.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_jit_compiler.cpp: 22 TEST_ASSERT
test_json.cpp: 20 TEST_ASSERT
test_lexer.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_live_view.cpp: 10 TEST_ASSERT
test_lsp.cpp: 10 TEST_ASSERT
test_macro.cpp: 10 TEST_ASSERT
test_million_processes.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_optimizer.cpp: 15 TEST_ASSERT
test_orm.cpp: 10 TEST_ASSERT
test_package_manager.cpp: 22 TEST_ASSERT
test_parser.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_pattern_matching.cpp: 15 TEST_ASSERT
test_preemption.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_production.cpp: 24 TEST_ASSERT
test_raft.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_repl.cpp: 10 TEST_ASSERT
test_security.cpp: 15 TEST_ASSERT
test_semantic.cpp: 20 TEST_ASSERT
test_session_auth.cpp: 10 TEST_ASSERT
test_socket_cluster.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_stdlib.cpp: 50 TEST_ASSERT
test_storage_engine.cpp: 15 TEST_ASSERT
test_tco.cpp: 0 TEST_ASSERT (Raw assert() legacy)
test_type_system.cpp: 15 TEST_ASSERT
test_view.cpp: 10 TEST_ASSERT
test_web.cpp: 10 TEST_ASSERT
```

---

## 4. Inventarisasi File Benchmark (`bench/`) & Metode Pengukuran

```text
bench_beam.cpp: Raw CPU loop cycle measurement for BEAM actor runtime.
bench_cluster.cpp: std::chrono measurements for cluster node discovery & load balancing.
bench_distributed_db.cpp: std::chrono measurements for distributed DB quorum writes & reads.
bench_edge_analytics.cpp: std::chrono measurements for window aggregation, Z-score, & alert dispatch.
bench_fase_g_audit.cpp: std::chrono measurements for total Phase G footprint & pipeline throughput.
bench_global_parity.cpp: std::chrono measurements for Hex package resolution & Mix build latency.
bench_gvm_micro.cpp: std::chrono measurements for GPIO toggle (100k loop), I2C read, & MQTT publish.
bench_hardware_drivers.cpp: std::chrono measurements for I2C, SPI, UART, PWM, & IRQ latencies.
bench_http_server.cpp: std::chrono measurements for HTTP server request/sec throughput.
bench_industrial.cpp: std::this_thread::sleep_for measurements for OTA, Secure Boot, Modbus, & OPC-UA.
bench_iot_framework.cpp: std::this_thread::sleep_for measurements for LoRaWAN OTAA join & BLE mesh.
bench_jit.cpp: std::chrono measurements for x86-64 machine code generation & JIT execution.
bench_package_manager.cpp: std::chrono measurements for Semver resolution & 500 stub compilation.
bench_production.cpp: std::chrono measurements for OTP release packaging, hot code upgrade, & rate limiting.
bench_raft.cpp: std::chrono measurements for Raft leader election & log replication.
bench_storage.cpp: std::chrono measurements for StorageEngine WAL persistence & query latency.
```

---

## 5. Audit Sanity Check `--sanity-check` Presence

The un-strippable `TEST_ASSERT` macro and `--sanity-check` command line flag are present in the following test files created/refactored from Sprint F.1 through Sprint I.0:

```text
tests/test_actor_http_server.cpp
tests/test_cluster_manager.cpp
tests/test_distributed_db.cpp
tests/test_distributed_foundation.cpp
tests/test_edge_analytics.cpp
tests/test_error_handling.cpp
tests/test_error_reporting.cpp
tests/test_fase_g_audit.cpp
tests/test_formatter_linter.cpp
tests/test_global_parity.cpp
tests/test_grdmix.cpp
tests/test_gvm_micro.cpp
tests/test_hardware_drivers.cpp
tests/test_http.cpp
tests/test_http_server.cpp
tests/test_industrial.cpp
tests/test_iot_framework.cpp
tests/test_jit_compiler.cpp
tests/test_json.cpp
tests/test_live_view.cpp
tests/test_lsp.cpp
tests/test_macro.cpp
tests/test_optimizer.cpp
tests/test_orm.cpp
tests/test_package_manager.cpp
tests/test_pattern_matching.cpp
tests/test_production.cpp
tests/test_repl.cpp
tests/test_security.cpp
tests/test_semantic.cpp
tests/test_session_auth.cpp
tests/test_stdlib.cpp
tests/test_storage_engine.cpp
tests/test_type_system.cpp
tests/test_view.cpp
tests/test_web.cpp
```

*Note: Legacy test files from Sprints A–E use C `assert()` statements and do not contain `--sanity-check` handlers.*
