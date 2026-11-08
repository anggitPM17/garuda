# garuda_csv_tabel_v4

> **Modul:** `CsvTabelV4` • **Kategori:** I/O & Format

Parser dan serializer berkas CSV/TSV streaming hemat memori.

## Alasan Pengayaan Ekosistem Garuda
Mempermudah import/export data tabular dan analisis data.

## Instalasi via GPM
```bash
gpm pasang garuda_csv_tabel_v4
```

## Contoh Penggunaan
```garuda
impor CsvTabelV4

publik fungsi utama(): Bulat {
    konst cfg = CsvTabelV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.