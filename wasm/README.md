# 🌐 Garuda BEAM WebAssembly (WASM) Target Guide

Dokumen ini menjelaskan panduan kompilasi **Garuda BEAM C++ Runtime ke WebAssembly (WASM)** menggunakan Emscripten.

---

## 🛠️ 1. Prasyarat Emscripten Toolchain

Instalasi `emsdk` di Linux/macOS/Windows:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

---

## 🏗️ 2. Kompilasi WASM

Jalankan script kompilasi:
```bash
chmod +x scripts/build_wasm.sh
./scripts/build_wasm.sh
```

Output kompilasi:
- `site/wasm/garuda.js` (Emscripten JavaScript glue)
- `site/wasm/garuda.wasm` (Compiled WebAssembly Binary Engine)

---

## 🌐 3. Penggunaan di HTML / JavaScript

```html
<script src="wasm/garuda.js"></script>
<script>
  createGarudaModule().then(module => {
    console.log("Garuda Version:", module.version());
    const output = module.run("modul Utama\nfungsi utama() ->\n    cetak(\"Halo WASM\")\nrampung");
    console.log("Console Output:\n", output);
  });
</script>
```
