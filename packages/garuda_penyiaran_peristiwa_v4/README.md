# garuda_penyiaran_peristiwa_v4

> **Modul:** `PenyiaranPeristiwaV4` • **Kategori:** Pola Desain & Aktor

Sistem Pub/Sub terdesentralisasi berbasis aktor penerima pesan.

## Alasan Pengayaan Ekosistem Garuda
Komunikasi reaktif asinkron antar komponen sistem.

## Instalasi via GPM
```bash
gpm pasang garuda_penyiaran_peristiwa_v4
```

## Contoh Penggunaan
```garuda
impor PenyiaranPeristiwaV4

publik fungsi utama(): Bulat {
    konst cfg = PenyiaranPeristiwaV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.