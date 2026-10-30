# 📦 GPM REGISTRY — AUDIT TRANSPARANSI STATUS PACKAGE (VERIFIED)

Dokumen ini mencatat hasil audit kode sumber dan status ketersediaan aktual dari 14 modul standar **Garuda Package Manager (GPM)** pada repositori [anggitPM17/garuda](https://github.com/anggitPM17/garuda).

---

## 📊 Klasifikasi Status Package

1. 💻 **`source_only`**: Kode C++ dan unit test tersedia di repositori `include/garuda/`, `src/`, dan `tests/`. Dapat diikutsertakan via header C++, namun belum tersedia sebagai installer independen `.gru` via CLI `gpm`.
2. ⏳ **`coming_soon`**: Modul dalam tahap perancangan dan akan hadir pada rilis GVM berikutnya.
3. ✅ **`available`**: Package biner siap diinstal langsung menggunakan CLI `gpm install`.
4. ⚠️ **`deprecated`**: Package tidak lagi dipelihara.

---

## 📋 Hasil Audit Per-Package

| No | Nama Package | Versi | Status Jujur | Kode Sumber / Test Case Terverifikasi |
| :-: | :--- | :-: | :-: | :--- |
| 1 | **`garuda_std`** | 1.0.0 | 💻 `source_only` | `include/garuda/beam/core/term.hpp` (Tipe Term & Aktor) |
| 2 | **`garuda_json`** | 1.2.0 | 💻 `source_only` | `include/garuda/beam/stdlib/json.hpp`, `tests/test_json.cpp` |
| 3 | **`garuda_regex`** | 1.1.0 | ⏳ `coming_soon` | Modul `regex` khusus dalam tahap pengembangan |
| 4 | **`garuda_datetime`** | 1.0.2 | ⏳ `coming_soon` | Modul `datetime` khusus dalam tahap pengembangan |
| 5 | **`garuda_http`** | 2.1.0 | 💻 `source_only` | `include/garuda/beam/stdlib/http_server.hpp`, `tests/test_http.cpp` |
| 6 | **`garuda_ws`** | 1.0.5 | 💻 `source_only` | `include/garuda/beam/net/websocket_handler.hpp`, `tests/test_websocket_e2e.cpp` |
| 7 | **`garuda_network`** | 1.0.1 | 💻 `source_only` | `include/garuda/beam/net/socket_layer.hpp` |
| 8 | **`garuda_sqlite`** | 1.3.0 | 💻 `source_only` | `src/vendor/sqlite3/sqlite3.h`, `tests/test_sqlite.cpp` |
| 9 | **`garuda_mnesia`** | 2.0.0 | 💻 `source_only` | `include/garuda/beam/otp/mnesia.hpp`, `tests/test_mnesia.cpp` |
| 10 | **`garuda_crypto`** | 3.0.1 | 💻 `source_only` | `include/garuda/beam/stdlib/crypto.hpp`, `tests/test_crypto.cpp` |
| 11 | **`garuda_jwt`** | 1.0.0 | 💻 `source_only` | `include/garuda/beam/stdlib/jwt.hpp`, `tests/test_jwt.cpp` |
| 12 | **`garuda_logger`** | 1.1.0 | 💻 `source_only` | `include/garuda/sys/json_logger.hpp`, `tests/test_logger.cpp` |
| 13 | **`garuda_cli`** | 1.0.0 | 💻 `source_only` | `src/gvm_admin.cpp` |
| 14 | **`garuda_test`** | 1.0.0 | 💻 `source_only` | `tests/test_beam.cpp` test suite framework |

---

🌐 **Katalog Live**: [https://richat.my.id/packages](https://richat.my.id/packages)
