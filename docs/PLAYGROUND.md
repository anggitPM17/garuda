# 🏟️ Stadion Garuda Web Playground Guide

Dokumen ini menjelaskan penggunaan **Stadion Garuda Web Playground** untuk menulis, mengompilasi, dan mengeksekusi sintaks kode Garuda `.grd` secara langsung.

---

## 🚀 Fitur Web Playground

- **Editor Kode Interaktif**: Menulis kode bahasa Garuda (`.grd`).
- **Kompilasi Real-time**: Terhubung ke Stadion Garuda Compiler Backend (`http://localhost:3001/api/run`) dengan *client-side fallback interpreter*.
- **GVM Execution Output**: Menampilkan hasil eksekusi AST, bytecode, dan cetakan konsol.

---

## 💻 Penggunaan

1. Akses halaman [site/playground.html](../site/playground.html) di browser.
2. Tulis kode sintaks Garuda pada editor:
   ```garuda
   modul Utama

   fungsi utama() ->
       cetak("Halo dari Stadion Garuda BEAM v67.0.0! 🦅🇮🇩")
   rampung
   ```
3. Klik **`⚡ Kompilasi & Jalankan (Run)`**.
4. Hasil eksekusi akan ditampilkan pada panel konsol di sebelah kanan.
