# garuda_web_uri_pembangun

> **Modul:** `UriPembangun` • **Kategori:** Web & Template • **Versi:** `0.1.0`

Parser dan pembuat URI terstandarisasi RFC 3986 dengan query parameter encoder.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Web & Template.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_web_uri_pembangun = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_web_uri_pembangun
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor UriPembangun

publik fungsi utama(): Bulat {
    konst instansi = UriPembangun::inisialisasi()
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