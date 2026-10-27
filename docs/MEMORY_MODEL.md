# 🧠 SPESIFIKASI SPASIAL & DESAIN MEMORY MODEL GARUDA

Dokumen spesifikasi arsitektur memori hibrida (*Hybrid Memory Architecture*) untuk Garuda Virtual Machine (GVM).

---

## 🎯 1. Latar Belakang & Filosofi Hibrida

Bahasa pemrograman tradisional menghadapi dilema arsitektur memori:
- **GC Tracing Global (Java, Go, C#)**: Sangat baik untuk backend terdistribusi, namun memicu *Stop-The-World (STW) GC pause* yang tidak dapat ditoleransi oleh sistem *real-time* seperti *game engine* dan aplikasi *mobile*.
- **Manual / ARC (C++, Rust, Swift)**: Performa deterministik tanpa jeda, namun kompleks untuk mengelola ribuan proses konkuren independen berbasis model aktor.

### Solusi Hibrida Garuda
Garuda menggabungkan dua paradigma memori terbaik dalam satu runtime sejak Lapis 0:
1. **Per-Actor Heap dengan Copying GC (GVM Backend Mode)**: Setiap aktor memiliki memori terisolasi. Jeda GC terisolasi per-aktor (< 1 ms), dan pembersihan memori instan $O(1)$ saat aktor dihancurkan.
2. **Arena / Frame Allocator Deterministik (Hot Path Game & Mobile Mode)**: *Bump-pointer allocation* tanpa GC pause untuk alur kerja yang membutuhkan latensi mikrodetik atau *fixed frame-rate* (60 / 120 FPS).

---

## 🏗️ 2. Arsitektur Memori GVM

```text
+-----------------------------------------------------------------------+
|                         GVM PROCESS MEMORY                            |
+-----------------------------------------------------------------------+
|                                                                       |
|  +------------------------+             +--------------------------+  |
|  |   ACTOR PROCESS A      |             |     ACTOR PROCESS B      |  |
|  |  +------------------+  |             |  +--------------------+  |  |
|  |  | Local Heap (GC)  |  |             |  | Arena Allocator    |  |  |
|  |  | Young / Old Gen  |  |             |  | (Bump-pointer)     |  |  |
|  |  +------------------+  |             |  +--------------------+  |  |
|  +------------|-----------+             +--------------|-----------+  |
|               | (Ref)                                  | (Ref)        |
|               v                                        v              |
|  +-----------------------------------------------------------------+  |
|  |              GLOBAL OFF-HEAP BINARY POOL                        |  |
|  |              (Ref-Counted Atomic Heap > 64 Bytes)               |  |
|  +-----------------------------------------------------------------+  |
+-----------------------------------------------------------------------+
```

---

## 🧹 3. Per-Actor Isolated Heap (Copying GC)

### Prinsip Utama
- **Zero Global Stop-The-World**: GC berjalan secara independen hanya pada aktor yang membutuhkan alokasi memori. Aktor lain terus berjalan tanpa terganggu.
- **Pembersihan Instan**: Saat aktor selesai (*exit/done*), seluruh *Local Heap* dibebaskan sekaligus tanpa perlu memanggil destruktor elemen satu per satu.
- **Copying Collector**: Menggunakan algoritma *Cheney's Copying Collector* (membagi heap menjadi *From-Space* dan *To-Space*) untuk memadatkan memori dan menghilangkan fragmentasi.

---

## ⚡ 4. Arena Allocator Deterministik (Hot Path)

### Kapan Digunakan?
- **Game Loop / Rendering Frame**: Alokasi objek per-frame yang akan dibebaskan sekaligus pada akhir frame (`arena_reset`).
- **NATIVE / Mobile Hot Path**: Jalur pemrosesan data real-time dengan garansi nol jeda alokator sistem OS.

### Karakteristik
- Alokasi cepat $O(1)$ menggunakan *Bump Pointer*.
- Tanpa instruksi `free()` individual — pembebasan memori seluruh arena dilakukan sekaligus melalui `arena_reset()`.

---

## 📦 5. Global Off-Heap Binary Pool (Shared Binaries)

- Data biner atau string besar (> 64 byte) diisolasi di luar *Actor Heap* ke dalam *Global Off-Heap Binary Pool*.
- Menggunakan *Atomic Reference Counting (ARC)*.
- Mengirimkan string/binari besar antar-aktor hanya mengirimkan *pointer reference* $O(1)$ tanpa menyalin byte data.

---

## 🔌 6. API Minimum Level Desain (C++ Header Spec)

```cpp
namespace garuda::beam::memory {

// Struct Representasi Arena Allocator
struct Arena {
    std::uint8_t* buffer = nullptr;
    std::size_t capacity = 0;
    std::size_t offset = 0;
};

// --- Actor Heap API ---
void* actor_alloc(void* process_ptr, std::size_t size);
void actor_gc_collect(void* process_ptr);
void actor_heap_destroy(void* process_ptr);

// --- Arena Allocator API ---
Arena* arena_create(std::size_t capacity);
void* arena_alloc(Arena* arena, std::size_t size, std::size_t alignment = 8);
void arena_reset(Arena* arena);
void arena_destroy(Arena* arena);

// --- Global Shared Binary Pool API ---
void* binary_alloc(std::size_t size);
void binary_retain(void* binary_ptr);
void binary_release(void* binary_ptr);

} // namespace garuda::beam::memory
```

---

## 🗺️ 7. Rencana Tahap Implementasi

1. **Tahap 1 (Lapis 0 - Sekarang)**: Spesifikasi API & *stub header* di `include/garuda/beam/core/memory.hpp`.
2. **Tahap 2 (Lapis 0 - Sprint Berikutnya)**: Implementasi *Arena / Frame Allocator* deterministik untuk pemrosesan hot path.
3. **Tahap 3 (Lapis 0 - Akhir)**: Integrasi *Per-Actor Copying Collector* ke dalam struct `GvmActor` di runtime.
