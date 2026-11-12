# garuda_kompresi_lz4_bingkai_submikro

> **Modul:** `Lz4BingkaiSubmikro` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Kompresor LZ4 berfokus pada kecepatan transfer data mendekati kecepatan RAM.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_lz4_bingkai_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_lz4_bingkai_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Lz4BingkaiSubmikro

publik fungsi utama(): Bulat {
    konst instansi = Lz4BingkaiSubmikro::inisialisasi()
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