# Language Server Protocol (LSP) Server untuk Bahasa Garuda (`garuda-lsp`)

LSP Server berbasis Node.js/TypeScript untuk Bahasa Garuda v67.0.0.

## Fitur LSP

- `textDocument/completion`: Autocomplete 21 keyword & 47 fungsi pustaka standar.
- `textDocument/hover`: Hover documentation dengan format Markdown.
- `textDocument/publishDiagnostics`: Real-time syntax check untuk bracket matching & unterminated strings.

## Build & Test

```powershell
cd lsp
npm install
npm run compile
node test/test_diagnostics.js
```
