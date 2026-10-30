# 📋 AUDIT DUKUNGAN LIST & MAP GVM (.grd & C++ STDLIB)

Dokumen ini mencatat hasil audit teknis terhadap dukungan operasi List dan Map pada compiler, VM interpreter, dan C++ Standard Library **Garuda BEAM**.

---

## 📊 Hasil Audit Operasi List & Map

| Fungsi .grd | Didukung Native C++? | Path C++ Implementation | Mekanisme Eksekusi |
| :--- | :---: | :--- | :--- |
| **`List.panjang(L)`** | ✅ | `include/garuda/beam/stdlib/list.hpp` (`List::panjang`) | Native VM / ScriptEngine Dispatch |
| **`List.kepala(L)`** | ✅ | `include/garuda/beam/stdlib/list.hpp` (`List::kepala`) | Native VM / ScriptEngine Dispatch |
| **`List.ekor(L)`** | ✅ | `include/garuda/beam/stdlib/list.hpp` (`List::ekor`) | Native VM / ScriptEngine Dispatch |
| **`List.tambah(L, X)`** | ✅ | `include/garuda/beam/stdlib/list.hpp` (`List::tambah`) | Native VM / ScriptEngine Dispatch |
| **`List.kosong(L)`** | ✅ | `include/garuda/beam/stdlib/list.hpp` (`List::kosong`) | Native VM / ScriptEngine Dispatch |
| **`Map.baru()`** | ✅ | `include/garuda/beam/stdlib/map.hpp` (`Map::baru`) | Native VM / ScriptEngine Dispatch |
| **`Map.taruh(M, K, V)`** | ✅ | `include/garuda/beam/stdlib/map.hpp` (`Map::taruh`) | Native VM / ScriptEngine Dispatch |
| **`Map.ambil(M, K)`** | ✅ | `include/garuda/beam/stdlib/map.hpp` (`Map::ambil`) | Native VM / ScriptEngine Dispatch |
| **`Map.hapus(M, K)`** | ✅ | `include/garuda/beam/stdlib/map.hpp` (`Map::hapus`) | Native VM / ScriptEngine Dispatch |

---

## 🛠️ Strategi Implementasi G3 (`std/list.grd` & `std/map.grd`)

1. **List Operations**:
   * Pattern matching `[H | T]` didukung di VM (`OpGetTupleElem` / `OpListHeadTail`).
   * Fungsi helper `List.panjang`, `List.kepala`, `List.ekor`, dan `List.tambah` dilayani oleh handler `ScriptEngine` native built-in dispatch yang memanggil `garuda::beam::stdlib::List`.

2. **Map Operations**:
   * Fungsi `Map.baru`, `Map.taruh`, `Map.ambil`, dan `Map.hapus` dilayani oleh handler `ScriptEngine` native built-in dispatch yang memanggil `garuda::beam::stdlib::Map`.
