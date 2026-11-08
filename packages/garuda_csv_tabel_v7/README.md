# garuda_csv_tabel_v7

> **Modul:** `CsvTabelV7` • **Kategori:** I/O & Format

Parser dan serializer berkas CSV/TSV streaming hemat memori.

## Alasan Pengayaan Ekosistem Garuda
Mempermudah import/export data tabular dan analisis data.

## Instalasi via GPM
```bash
gpm pasang garuda_csv_tabel_v7
```

## Contoh Penggunaan
```garuda
impor CsvTabelV7

publik fungsi utama(): Bulat {
    konst cfg = CsvTabelV7::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.