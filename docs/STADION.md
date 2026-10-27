# 🏟️ DESAIN ARSITEKTUR STADION GARUDA (PLAYGROUND INTERAKTIF WASM)

Dokumen perancangan *Stadion Garuda* (Playground Interaktif WebAssembly) untuk Lapis 0.5.

---

## 🎯 1. Visi & Tujuan Stadion Garuda

*Stadion Garuda* adalah lingkungan eksekusi interaktif di peramban web (*browser*) yang memungkinkan pengguna mencoba, mempelajari, dan memvisualisasikan eksekusi kode Bahasa Garuda (model aktor, TCO, scheduler) secara langsung tanpa perlu memasang compiler `grdc` di mesin lokal.

---

## 🏗️ 2. Arsitektur Kompilasi WebAssembly (Emscripten)

```text
+-----------------------------------------------------------------------+
|                         BROWSER CLIENT SIDE                           |
+-----------------------------------------------------------------------+
|                                                                       |
|  +---------------------------+        +----------------------------+  |
|  |   Monaco / CodeMirror     |        |    GVM Engine WebAssembly  |  |
|  |   Editor (Sintaks .grd)   |        |    (Garuda GVM in WASM)    |  |
|  +-------------|-------------+        +--------------^-------------+  |
|                | Source Code                         | Bytecode       |
|                v                                     |                |
|  +---------------------------------------------------|-------------+  |
|  |           grdc Compiler in WASM (In-Browser Compile)            |  |
|  +-----------------------------------------------------------------+  |
+-----------------------------------------------------------------------+
```

### Komponen Utama
1. **Garuda Compiler (`grdc.wasm`)**: Porting C++ compiler Garuda ke WebAssembly menggunakan Emscripten SDK (`emcc`).
2. **GVM Runtime Engine (`gvm.wasm`)**: Runtime interpreter ringan C++ yang dieksekusi di WebAssembly worker thread.
3. **Web UI Visualizer**: UI frontend modern berbasis HTML5/JS untuk menyajikan konsol log, state registri aktor, dan struktur memori secara visual.

---

## 🚀 3. Target Fitur Lapis 0.5

- [ ] **Kompilasi In-Browser**: Mengompilasi kode `.grd` langsung ke biner `.gvm` di memori peramban (< 20 ms).
- [ ] **Visualisasi Aktor**: Menampilkan daftar aktor aktif, antrean pesan (*mailbox*), dan status siklus preemption.
- [ ] **Pencegahan Infinite Loop**: Menggunakan anggaran *reductions counter* GVM untuk menghentikan program yang masuk *infinite loop* secara aman.
