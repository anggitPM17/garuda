# garuda_regex_pola_v8

> **Modul:** `RegexPolaV8` • **Kategori:** Parsing & Teks

Mesin ekspresi reguler deterministik (DFA/NFA) murni tanpa alokasi berat.

## Alasan Pengayaan Ekosistem Garuda
Diperlukan untuk validasi formulir dan ekstraksi token compiler.

## Instalasi via GPM
```bash
gpm pasang garuda_regex_pola_v8
```

## Contoh Penggunaan
```garuda
impor RegexPolaV8

publik fungsi utama(): Bulat {
    konst cfg = RegexPolaV8::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.