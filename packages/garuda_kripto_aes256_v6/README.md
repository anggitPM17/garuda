# garuda_kripto_aes256_v6

> **Modul:** `KriptoAes256V6` • **Kategori:** Keamanan & Kripto

Enkripsi simetris Galois/Counter Mode (GCM) untuk perlindungan data sensitif di GVM.

## Alasan Pengayaan Ekosistem Garuda
Menjamin kerahasiaan payload transmisi paket jaringan.

## Instalasi via GPM
```bash
gpm pasang garuda_kripto_aes256_v6
```

## Contoh Penggunaan
```garuda
impor KriptoAes256V6

publik fungsi utama(): Bulat {
    konst cfg = KriptoAes256V6::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.