# garuda_format_hex_biner

> **Modul:** `HexBiner` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Konversi heksadesimal ke byte buffer dan sebaliknya dengan validasi ketat.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Parsing & Format Data.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_hex_biner = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_hex_biner
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor HexBiner

publik fungsi utama(): Bulat {
    konst instansi = HexBiner::inisialisasi()
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