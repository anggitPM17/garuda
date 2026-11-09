# garuda_kompresi_deflate_aliran

> **Modul:** `DeflateAliran` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Mesin kompresi DEFLATE RFC 1951 menggabungkan LZ77 dan pengkodean Huffman.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kompresi & Berkas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_deflate_aliran = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_deflate_aliran
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor DeflateAliran

publik fungsi utama(): Bulat {
    konst instansi = DeflateAliran::inisialisasi()
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