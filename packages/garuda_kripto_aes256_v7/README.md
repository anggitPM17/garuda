# garuda_kripto_aes256_v7

> **Modul:** `KriptoAes256V7` • **Kategori:** Keamanan & Kripto

Enkripsi simetris Galois/Counter Mode (GCM) untuk perlindungan data sensitif di GVM.

## Alasan Pengayaan Ekosistem Garuda
Menjamin kerahasiaan payload transmisi paket jaringan.

## Instalasi via GPM
```bash
gpm pasang garuda_kripto_aes256_v7
```

## Contoh Penggunaan
```garuda
impor KriptoAes256V7

publik fungsi utama(): Bulat {
    konst cfg = KriptoAes256V7::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.