# 🛠️ Contoh CLI Tool Garuda: `salam`

Project ini adalah contoh aplikasi Command Line Interface (CLI) sederhana yang dibuat menggunakan bahasa Garuda dan berjalan di atas Garuda Virtual Machine (GVM).

---

## 🚀 Cara Build & Run

### 1. Menggunakan `grdmix` (Package Manager Garuda)
```bash
grdmix compile
grdmix run
```

### 2. Kompilasi Manual dengan `grdc`
```bash
grdc main.grd -o salam.gvm
gvm salam.gvm --nama Budi
```

---

## 📌 Contoh Output

```bash
$ gvm salam.gvm --nama Budi
Halo, Budi!

$ gvm salam.gvm --help
Penggunaan: salam --nama <nama>
```
