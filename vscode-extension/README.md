# VS Code Extension untuk Bahasa Garuda (`garuda-vscode-lsp`)

Ekstensi VS Code resmi untuk Bahasa Garuda yang menyediakan Language Server Protocol (LSP) Client untuk autocomplete, hover documentation, dan diagnostik sintaks.

## Fitur

- **Autocomplete**: Autocomplete untuk 21 kata kunci Garuda dan 47 fungsi pustaka standar (`List.*`, `String.*`, `Map.*`, `Tuple.*`, `Waktu.*`, `Matematika.*`, `Http.*`, `Web.*`).
- **Hover Documentation**: Menampilkan signature tipe dan dokumentasi Markdown saat kursor diarahkan ke atas nama fungsi atau kata kunci.
- **Diagnostik Sintaks**: Memeriksa penutupan kurung `()`, `[]`, `{}` dan string literal yang tidak ditutup secara real-time.

## Cara Pemasangan Development Mode

1. Salin direktori `vscode-extension` dan `lsp` ke folder ekstensi VS Code:
   ```powershell
   # Compile LSP server & client
   cd lsp; npm install; npm run compile
   cd ../vscode-extension; npm install; npm run compile

   # Salin ke ekstensi pengguna
   Copy-Item -Recurse . "$env:USERPROFILE\.vscode\extensions\garuda-vscode-lsp-0.1.0"
   ```
2. Muat ulang / restart VS Code (`Ctrl+Shift+P` -> `Developer: Reload Window`).
3. Buka berkas skrip Garuda (`.grd`).
