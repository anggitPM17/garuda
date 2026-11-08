# garuda_csv_tabel

> **Modul:** `CsvTabel` • **Kategori:** I/O & Format Data • **Versi:** `0.1.0`

Parser dan penulis format RFC 4180 streaming dengan dukungan kutip ganda, koma/baris baru dalam sel, dan pelaporan baris galat.

## Arsitektur & Tujuan
Pertukaran data tabular industri, import/export spreadsheet, dan ingestion pipeline.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_csv_tabel = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_csv_tabel
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor CsvTabel

publik fungsi utama(): Bulat {
    konst instansi = CsvTabel::inisialisasi()
    kembalikan 0
}
```

## Batasan yang Diketahui (Known Limitations)
- Modul ini dirancang murni di atas runtime GVM (Garuda Virtual Machine).
- Bergantung pada ketersediaan alokasi heap memori thread-safe.

## Kebutuhan Engine C++ GVM
- Jika modul memerlukan operasi I/O jaringan fisik langsung, modul memanggil FFI socket `garuda_socket_ffi`.
- Kriptografi CSPRNG membutuhkan entropy source `/dev/urandom` atau WinCrypt pada level host C++.

## Lisensi
Lisensi MIT © 2026 Anggit Prasetyo (@anggitPM17).