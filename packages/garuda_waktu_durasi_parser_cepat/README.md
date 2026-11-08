# garuda_waktu_durasi_parser_cepat

> **Modul:** `DurasiParserCepat` • **Kategori:** Waktu & Kalender • **Versi:** `0.1.0`

Varian Cepat berkinerja tinggi: Parser teks durasi manusia seperti '2j 30m 15d' menjadi nilai milidetik.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur cepat untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_waktu_durasi_parser_cepat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_waktu_durasi_parser_cepat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor DurasiParserCepat

publik fungsi utama(): Bulat {
    konst instansi = DurasiParserCepat::inisialisasi()
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