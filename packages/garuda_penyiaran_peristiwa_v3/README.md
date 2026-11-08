# garuda_penyiaran_peristiwa_v3

> **Modul:** `PenyiaranPeristiwaV3` • **Kategori:** Pola Desain & Aktor

Sistem Pub/Sub terdesentralisasi berbasis aktor penerima pesan.

## Alasan Pengayaan Ekosistem Garuda
Komunikasi reaktif asinkron antar komponen sistem.

## Instalasi via GPM
```bash
gpm pasang garuda_penyiaran_peristiwa_v3
```

## Contoh Penggunaan
```garuda
impor PenyiaranPeristiwaV3

publik fungsi utama(): Bulat {
    konst cfg = PenyiaranPeristiwaV3::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.