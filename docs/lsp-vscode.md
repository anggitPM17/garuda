# Language Server Protocol (LSP) & Ekstensi VS Code

Panduan pemasangan dan fitur Language Server Protocol (`garuda-lsp`) dan Ekstensi VS Code (`garuda-vscode-lsp`) untuk Bahasa Garuda v67.0.0.

---

## 1. Fitur Utama Tooling IDE

- **✨ Autocomplete (68 Simbol)**: Mendukung penyelesaian otomatis untuk 21 kata kunci bahasa Garuda (`fungsi`, `modul`, `yen`, `nalika`, `cakra`, `tampa`, dll.) serta 47 fungsi pustaka standar (`List.*`, `String.*`, `Map.*`, `Tuple.*`, `Waktu.*`, `Matematika.*`, `Http.*`, `Web.*`).
- **💡 Hover Documentation**: Menampilkan dokumentasi Markdown serta signature tipe fungsi saat kursor mengarah ke kata kunci atau nama fungsi.
- **🔍 Diagnostik Sintaks Real-Time**: Memeriksa penutupan kurung `()`, `[]`, `{}` serta mendeteksi string literal yang tidak ditutup.

---

## 2. Pemasangan Ekstensi di VS Code

### Kompilasi Ekstensi

```powershell
# 1. Kompilasi LSP Server
cd lsp
npm install
npm run compile

# 2. Kompilasi VS Code Client Extension
cd ../vscode-extension
npm install
npm run compile
```

### Pemasangan ke Directory VS Code User

```powershell
# Salin direktori ke folder ekstensi VS Code pengguna
Copy-Item -Recurse . "$env:USERPROFILE\.vscode\extensions\garuda-vscode-lsp-0.1.0"
```

Muat ulang VS Code (`Ctrl+Shift+P` $\rightarrow$ `Developer: Reload Window`). Saat berkas `.grd` dibuka, ekstensi dan LSP server akan aktif secara otomatis.

---

## 3. Integrasi dengan Editor Lain (Neovim / Emacs)

Server LSP mendukung komunikasi standar JSON-RPC melalui `stdio`:

```bash
node lsp/out/server.js --stdio
```

Konfigurasikan LSP client pada editor Neovim (`nvim-lspconfig`) atau Emacs (`lsp-mode`) untuk memanggil perintah Node.js di atas pada berkas berkategori `garuda`.
