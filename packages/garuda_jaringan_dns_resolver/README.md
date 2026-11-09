# garuda_jaringan_dns_resolver

> **Modul:** `DnsResolver` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Klien DNS UDP/TCP dengan penguraian record A, AAAA, CNAME, MX, dan TXT.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Jaringan & Protokol.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_dns_resolver = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_dns_resolver
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor DnsResolver

publik fungsi utama(): Bulat {
    konst instansi = DnsResolver::inisialisasi()
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