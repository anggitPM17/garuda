# garuda_basisdata_kv

> **Modul:** `BasisDataKv` • **Kategori:** Penyimpanan Data

Database key-value memory-mapped dengan Write-Ahead Log (WAL) otomatis.

## Alasan Pengayaan Ekosistem Garuda
Persistensi lokal berkecepatan mikrodetik untuk modul backend.

## Instalasi via GPM
```bash
gpm pasang garuda_basisdata_kv
```

## Contoh Penggunaan
```garuda
impor BasisDataKv

publik fungsi utama(): Bulat {
    konst cfg = BasisDataKv::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.