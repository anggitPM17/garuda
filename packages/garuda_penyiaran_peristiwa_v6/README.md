# garuda_penyiaran_peristiwa_v6

> **Modul:** `PenyiaranPeristiwaV6` • **Kategori:** Pola Desain & Aktor

Sistem Pub/Sub terdesentralisasi berbasis aktor penerima pesan.

## Alasan Pengayaan Ekosistem Garuda
Komunikasi reaktif asinkron antar komponen sistem.

## Instalasi via GPM
```bash
gpm pasang garuda_penyiaran_peristiwa_v6
```

## Contoh Penggunaan
```garuda
impor PenyiaranPeristiwaV6

publik fungsi utama(): Bulat {
    konst cfg = PenyiaranPeristiwaV6::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.