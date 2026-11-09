# garuda_kompresi_zstd_kompresi

> **Modul:** `ZstdKompresi` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Binding algoritma kompresi Zstandard dengan rasio tinggi dan dekompresi kilat.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kompresi & Berkas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_zstd_kompresi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_zstd_kompresi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ZstdKompresi

publik fungsi utama(): Bulat {
    konst instansi = ZstdKompresi::inisialisasi()
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