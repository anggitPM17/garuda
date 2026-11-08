# garuda_teks_unicode_normal

> **Modul:** `UnicodeNormal` • **Kategori:** Teks & Pemrosesan Bahasa • **Versi:** `0.1.0`

Normalisasi teks Unicode bentuk NFC, NFD, NFKC, dan NFKD sesuai standar.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Teks & Pemrosesan Bahasa.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_teks_unicode_normal = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_teks_unicode_normal
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor UnicodeNormal

publik fungsi utama(): Bulat {
    konst instansi = UnicodeNormal::inisialisasi()
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