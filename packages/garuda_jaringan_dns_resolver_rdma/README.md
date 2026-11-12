# garuda_jaringan_dns_resolver_rdma

> **Modul:** `DnsResolverRdma` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Varian Rdma berkinerja tinggi: Klien DNS UDP/TCP dengan penguraian record A, AAAA, CNAME, MX, dan TXT.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur rdma untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_dns_resolver_rdma = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_dns_resolver_rdma
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor DnsResolverRdma

publik fungsi utama(): Bulat {
    konst instansi = DnsResolverRdma::inisialisasi()
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