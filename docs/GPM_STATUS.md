# 📦 GPM REGISTRY — AUDIT TRANSPARANSI STATUS PACKAGE (VERIFIED)

Dokumen ini mencatat hasil audit kode sumber dan status ketersediaan aktual dari 19 modul standar **Garuda Package Manager (GPM)** pada repositori [anggitPM17/garuda](https://github.com/anggitPM17/garuda).

---

## 📊 Klasifikasi Status Package & Bahasa

1. 📜 **`source_only_grd`**: Library murni bahasa Garuda (`.grd`) terverifikasi lulus unit test.
2. 💻 **`source_only`**: Kode C++ dan unit test tersedia di repositori `include/garuda/`, `src/`, dan `tests/`.
3. ⏳ **`coming_soon`**: Modul dalam tahap perancangan.
4. ✅ **`available`**: Package biner siap diinstal langsung menggunakan CLI `gpm install`.

---

## 📋 Hasil Audit Per-Package

| No | Nama Package | Versi | Bahasa | Status Jujur | Kode Sumber / Test Case Terverifikasi |
| :-: | :--- | :-: | :-: | :-: | :--- |
| 1 | **`garuda_string_grd`** | 1.0.0 | Grd | 📜 `source_only_grd` | `std/string.grd`, `tests/test_string_grd.cpp` |
| 2 | **`garuda_result_grd`** | 1.0.0 | Grd | 📜 `source_only_grd` | `std/result.grd`, `tests/test_result_grd.cpp` |
| 3 | **`garuda_list_grd`** | 1.0.0 | Grd | 📜 `source_only_grd` | `std/list.grd`, `tests/test_list_grd.cpp` |
| 4 | **`garuda_map_grd`** | 1.0.0 | Grd | 📜 `source_only_grd` | `std/map.grd`, `tests/test_map_grd.cpp` |
| 5 | **`garuda_json_grd`** | 1.0.0 | Grd | 📜 `source_only_grd` | `std/json.grd`, `tests/test_json_grd.cpp` |
| 6 | **`garuda_std`** | 1.0.0 | C++ | 💻 `source_only` | `include/garuda/beam/core/term.hpp` |
| 7 | **`garuda_json`** | 1.2.0 | C++ | 💻 `source_only` | `include/garuda/beam/stdlib/json.hpp` |
| 8 | **`garuda_regex`** | 1.1.0 | C++ | ⏳ `coming_soon` | Modul `regex` khusus dalam tahap pengembangan |
| 9 | **`garuda_datetime`** | 1.0.2 | C++ | ⏳ `coming_soon` | Modul `datetime` khusus dalam tahap pengembangan |
| 10 | **`garuda_http`** | 2.1.0 | C++ | 💻 `source_only` | `include/garuda/beam/stdlib/http_server.hpp` |
| 11 | **`garuda_ws`** | 1.0.5 | C++ | 💻 `source_only` | `include/garuda/beam/net/websocket_handler.hpp` |
| 12 | **`garuda_network`** | 1.0.1 | C++ | 💻 `source_only` | `include/garuda/beam/net/socket_layer.hpp` |
| 13 | **`garuda_sqlite`** | 1.3.0 | C++ | 💻 `source_only` | `src/vendor/sqlite3/sqlite3.h` |
| 14 | **`garuda_mnesia`** | 2.0.0 | C++ | 💻 `source_only` | `include/garuda/beam/otp/mnesia.hpp` |
| 15 | **`garuda_crypto`** | 3.0.1 | C++ | 💻 `source_only` | `include/garuda/beam/stdlib/crypto.hpp` |
| 16 | **`garuda_jwt`** | 1.0.0 | C++ | 💻 `source_only` | `include/garuda/beam/stdlib/jwt.hpp` |
| 17 | **`garuda_logger`** | 1.1.0 | C++ | 💻 `source_only` | `include/garuda/sys/json_logger.hpp` |
| 18 | **`garuda_cli`** | 1.0.0 | C++ | 💻 `source_only` | `src/gvm_admin.cpp` |
| 19 | **`garuda_test`** | 1.0.0 | C++ | 💻 `source_only` | `tests/test_beam.cpp` |

---

🌐 **Katalog Live**: [https://richat.my.id/packages](https://richat.my.id/packages)
