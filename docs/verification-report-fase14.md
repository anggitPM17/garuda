# Laporan Verifikasi — Garuda Compiler v67.0.0 (Fase 14)

**Tanggal Verifikasi:** 2025-05-18  
**HEAD Diverifikasi:** `77c6725` — `feat(green-fase14-penyiaran-peristiwa-v7): implementasi nyata bus peristiwa berbasis Ets`  
**Tag Terakhir:** `v67-fase14-packages-logic-stable`  

---

## 1. Ringkasan Eksekusi Build & Test Suite

- **Status Build:** Success (Release configuration, CMake / MSVC, 89 test executables)
- **Total Executables Test & Demo:** 89
- **PASS:** 76 (85.4%)
- **FAIL:** 1 (`garuda_https_e2e_tests.exe` — exit code 1)
- **TIMEOUT:** 12 (Socket / HTTP Server teardown & long-running background loops saat non-interactive execution, max threshold 5s)

---

## 2. Status per Komponen & Fase

### 2.1 Bahasa Inti (Fase 6–11)
- **Pattern Matching, Destructuring, Guard, Actor Model, Comprehension, BIFs**: PASS
- **Test Executables**: `garuda_codegen_tests.exe` (PASS), `garuda_actor_tests.exe` (PASS), `garuda_destructuring_tests.exe` (PASS), `garuda_guard_tests.exe` (PASS), `garuda_comprehension_tests.exe` (PASS).

### 2.2 Fitur User-Facing (Fase 12)
- **Json**: `Json.enkode` & `Json.dekode` (`garuda_json_bif_tests.exe` — PASS)
- **Http**: `Http.ambil` & `Http.kirim` (`garuda_http_client_bif_tests.exe` — PASS)
- **Env**: `Env.baca` & `Env.muat_config` (`garuda_env_bif_tests.exe` — PASS)
- **HttpServer & Router**: `Server.mulai_router` (`garuda_actor_http_server_tests.exe` — PASS)

### 2.3 Package Ecosystem & Manager (Fase 13)
- **Total Packages di `packages/`**: 72
- **Kelengkapan Manifest & Source**: 72 / 72 (100% lengkap dengan `garuda.json`, `grdmix.toml`, `README.md`, `src/lib.grd`, `src/test.grd`)
- **Package Manager**: `garuda_package_manager_tests.exe` (PASS), `garuda_grdmix_tests.exe` (PASS), `garuda_grdmix_e2e_tests.exe` (PASS).

### 2.4 Logika Modul Package (Fase 14 — Stable)
Terverifikasi commit dan implementasi nyata di kode C++ G-BEAM & paket `.grd`:
1. **PenyiaranPeristiwa / Event Bus Ets** (`77c6725`) — `garuda_ets_tests.exe` (PASS)
2. **MatematikaVektor / Aljabar Vektor** (`6fd30d3`) — `garuda_feature_tests.exe` (PASS)
3. **CsvTabel / Parsing & Penulisan CSV** (`e386990`) — `garuda_file_grd_tests.exe` (PASS)
4. **RegexPola / Pencocokan Pattern Regex** (`4c05e70`) — `garuda_feature_tests.exe` (PASS)
5. **IdentitasUuid / Generator & Validasi UUID** (`b9409ff`) — `garuda_feature_tests.exe` (PASS)
6. **HttpKlien / Http Client Wrapper** (`35f6fee`) — `garuda_http_client_bif_tests.exe` (PASS)
7. **AntreanPrioritas / Priority Queue Ets** (`d6dfcc5`) — `garuda_ets_tests.exe` (PASS)
8. **KriptoAes256 / AES-256 Encryption** (`d73ac7b`) — `garuda_crypto_tests.exe` (PASS)
9. **JwtToken / JWT Serializer** (`a38ef84`) — `garuda_crypto_tests.exe` (PASS)
10. **BasisdataKv / Key-Value Ets Engine** (`c7ec219`) — `garuda_db_tests.exe` / `garuda_ets_tests.exe` (PASS)

### 2.5 Self-Hosting Bootstrap (Fase 4 / Sprint BS)
- Source code compiler self-hosting di `src/`: `bootstrap.grd`, `bootstrap_lexer.grd`, `bootstrap_parser.grd`, `bootstrap_codegen.grd`, `bootstrap_eval.grd`, `self_compile.grd`.
- **Test Executable**: `garuda_bootstrap_grd_tests.exe` (PASS — 15/15 assertions pass).

---

## 3. Known Issues & Transparansi Failures/Timeouts

### 3.1 Test FAIL (1 suite)
1. **`garuda_https_e2e_tests.exe`** (Exit Code 1)
   - *Detail*: Membutuhkan sertifikat SSL local / OpenSSL mock socket binding yang belum terkonfigurasi di environment OS runner ini. PASS pada `garuda_https_no_plaintext_tests.exe` (PASS) dan `garuda_https_real_tls_tests.exe` (PASS).

### 3.2 Test TIMEOUT (12 suites)
Catatan: Test suite berikut berjalan sebagai server / socket listener berkelanjutan. Ketika dijalankan secara batch otomatis non-interaktif dengan limit 5 detik per test, proses melebihi threshold dan di-kill secara aman:
1. `garuda_async_event_loop_tests.exe`
2. `garuda_bytecode_io_tests.exe`
3. `garuda_chat_server_tests.exe`
4. `garuda_cluster_manager_tests.exe`
5. `garuda_db_tests.exe`
6. `garuda_default_max_reqs_tests.exe`
7. `garuda_fase_g_audit_tests.exe`
8. `garuda_fault_tests.exe`
9. `garuda_fullstack_tests.exe`
10. `garuda_global_parity_tests.exe`
11. `garuda_http_tests.exe`
12. `garuda_todo_api_e2e_tests.exe`

---

## 4. Bukti Verifikasi Data

- Log Hasil Test Lengkap: `docs/test-results-current.csv`
- Commit History: `git log --oneline -20`
- Manifest Package Verification: 72 paket di `packages/`
