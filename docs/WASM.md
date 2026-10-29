# 🌐 Garuda BEAM WebAssembly (WASM) Target Guide (FASE D.3)

Dokumen ini menjelaskan arsitektur dan panduan penggunaan **Garuda BEAM WebAssembly (WASM) Target**.

---

## 🌟 Keunggulan WebAssembly Target

- **Zero Server Overhead**: Eksekusi kompilasi dan runtime bahasa Garuda `.grd` langsung di browser klien tanpa beban server backend.
- **Client-Side Sandbox**: Keamanan tinggi terisolasi sepenuhnya di dalam mesin V8 / SpiderMonkey browser.
- **Cross-Browser Compatible**: Berjalan mulus di Chrome, Firefox, Safari, Edge, dan WebViews seluler.

---

## 🏗️ Komponen Utama WASM Engine

1. **`garuda_wasm.cpp` (`wasm/garuda_wasm.cpp`)**: Entry point C++ C-linkage export `garuda_version()` dan `garuda_run(code)`.
2. **`bindings.cpp` (`wasm/bindings.cpp`)**: Emscripten `embind` bindings untuk ekspor modul JavaScript `createGarudaModule()`.
3. **`playground_wasm.html` (`site/playground_wasm.html`)**: Halaman Stadion Garuda WebAssembly Playground.
