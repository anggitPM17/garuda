# garuda_csv_tabel

> **Modul:** `CsvTabel` • **Kategori:** I/O & Format

Parser dan serializer berkas CSV/TSV streaming hemat memori.

## Alasan Pengayaan Ekosistem Garuda
Mempermudah import/export data tabular dan analisis data.

## Instalasi via GPM
```bash
gpm pasang garuda_csv_tabel
```

## Contoh Penggunaan
```garuda
impor CsvTabel

publik fungsi utama(): Bulat {
    konst cfg = CsvTabel::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.