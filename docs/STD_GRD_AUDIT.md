# 📜 AUDIT FILE MODUL `std/*.grd` (BAHASA GARUDA)

Dokumen ini mencatat hasil audit isi file terhadap seluruh 14 file `.grd` di folder `std/` pada repositori [anggitPM17/garuda](https://github.com/anggitPM17/garuda).

---

## 📊 Hasil Audit Isi Modul `std/*.grd`

| File `.grd` | Baris | Tipe Isi | Fungsi Utama | Status Kesiapan |
| :--- | :-: | :---: | :--- | :--- |
| **`std/json.grd`** | 14 | ⚠️ Stub | `encode()`, `decode()` | Placeholder stub yang mengembalikan argumen masukan. |
| **`std/string.grd`** | 29 | ⚠️ Stub | `panjang()`, `besar()`, `kecil()`, `potong()`, `gabung()` | Placeholder stub komentar. |
| **`std/list.grd`** | 24 | ⚠️ Stub | `panjang()`, `tambah()`, `kepala()`, `ekor()` | Placeholder stub komentar. |
| **`std/map.grd`** | 24 | ⚠️ Stub | `baru()`, `taruh()`, `ambil()`, `hapus()` | Placeholder stub. |
| **`std/db.grd`** | 42 | ⚠️ Stub | `open()`, `insert()`, `query()`, `update()`, `delete()` | Stub percabangan `yen`/`lain`. |
| **`std/http.grd`** | 18 | ⚠️ Stub | `get()`, `post()` | Stub percabangan `yen`/`lain`. |
| **`std/http_server.grd`** | 18 | ⚠️ Stub | `serve()`, `route()` | Stub percabangan `yen`/`lain`. |
| **`std/auth.grd`** | 14 | ⚠️ Stub | `login()`, `protect()` | Stub otentikasi token mock. |
| **`std/orm.grd`** | 46 | ⚠️ Stub | `define_model()`, `insert()`, `find()`, `update()`, `delete()` | Stub ORM mock. |
| **`std/result.grd`** | 39 | ⚠️ Stub | `is_ok()`, `is_error()`, `unwrap()`, `unwrap_or()`, `map()` | Stub pattern matching `tinjau`. |
| **`std/session.grd`** | 14 | ⚠️ Stub | `start()`, `get()`, `set()` | Stub session management. |
| **`std/view.grd`** | 18 | ⚠️ Stub | `render()`, `template()` | Stub template rendering. |
| **`std/web.grd`** | 30 | ⚠️ Stub | `dispatch()`, `json_response()` | Stub web routing. |
| **`std/live_view.grd`** | 26 | ⚠️ Stub | `mount()`, `handle_event()`, `render()` | Stub LiveView actor loop. |

---

> **Kesimpulan Audit `std/*.grd`:**  
> Seluruh 14 file `.grd` di folder `std/` saat ini merupakan **stub/placeholder** dengan fungsi mock.  
> Untuk rilis berikutnya, modul-modul ini dapat diisi dengan implementasi logika bahasa Garuda `.grd` yang sesungguhnya secara bertahap (dimulai dari `garuda_std` / `std/string.grd` & `std/json.grd`).
