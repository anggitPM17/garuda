# garuda_kompresi_bzip2_blok

> **Modul:** `Bzip2Blok` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Kompresor Burrows-Wheeler Transform dengan pemodelan probabilitas MTF.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kompresi & Berkas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_bzip2_blok = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_bzip2_blok
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Bzip2Blok

publik fungsi utama(): Bulat {
    konst instansi = Bzip2Blok::inisialisasi()
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