# garuda_csv_tabel_v5

> **Modul:** `CsvTabelV5` • **Kategori:** I/O & Format

Parser dan serializer berkas CSV/TSV streaming hemat memori.

## Alasan Pengayaan Ekosistem Garuda
Mempermudah import/export data tabular dan analisis data.

## Instalasi via GPM
```bash
gpm pasang garuda_csv_tabel_v5
```

## Contoh Penggunaan
```garuda
impor CsvTabelV5

publik fungsi utama(): Bulat {
    konst cfg = CsvTabelV5::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.