# garuda_regex_pola

> **Modul:** `RegexPola` • **Kategori:** Parsing & Teks • **Versi:** `0.1.0`

Mesin ekspresi reguler deterministik (Thompson NFA) waktu linier O(n), mendukung kelas karakter, kuantifier, dan anchor.

## Arsitektur & Tujuan
Mencegah catastrophic backtracking ReDoS pada validasi input formulir dan parser compiler.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_regex_pola = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_regex_pola
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RegexPola

publik fungsi utama(): Bulat {
    konst instansi = RegexPola::inisialisasi()
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