# garuda_regex_pola_v6

> **Modul:** `RegexPolaV6` • **Kategori:** Parsing & Teks

Mesin ekspresi reguler deterministik (DFA/NFA) murni tanpa alokasi berat.

## Alasan Pengayaan Ekosistem Garuda
Diperlukan untuk validasi formulir dan ekstraksi token compiler.

## Instalasi via GPM
```bash
gpm pasang garuda_regex_pola_v6
```

## Contoh Penggunaan
```garuda
impor RegexPolaV6

publik fungsi utama(): Bulat {
    konst cfg = RegexPolaV6::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.