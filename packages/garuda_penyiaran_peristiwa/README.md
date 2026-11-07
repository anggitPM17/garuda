# garuda_penyiaran_peristiwa

> **Modul:** `PenyiaranPeristiwa` • **Kategori:** Pola Desain & Aktor

Sistem Pub/Sub terdesentralisasi berbasis aktor penerima pesan.

## Alasan Pengayaan Ekosistem Garuda
Komunikasi reaktif asinkron antar komponen sistem.

## Instalasi via GPM
```bash
gpm pasang garuda_penyiaran_peristiwa
```

## Contoh Penggunaan
```garuda
impor PenyiaranPeristiwa

publik fungsi utama(): Bulat {
    konst cfg = PenyiaranPeristiwa::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.