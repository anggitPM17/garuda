# garuda_basisdata_kv_v4

> **Modul:** `BasisDataKvV4` • **Kategori:** Penyimpanan Data

Database key-value memory-mapped dengan Write-Ahead Log (WAL) otomatis.

## Alasan Pengayaan Ekosistem Garuda
Persistensi lokal berkecepatan mikrodetik untuk modul backend.

## Instalasi via GPM
```bash
gpm pasang garuda_basisdata_kv_v4
```

## Contoh Penggunaan
```garuda
impor BasisDataKvV4

publik fungsi utama(): Bulat {
    konst cfg = BasisDataKvV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.