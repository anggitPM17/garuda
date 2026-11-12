# garuda_jaringan_tls_jabat_tangan_rdma

> **Modul:** `TlsJabatTanganRdma` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Varian Rdma berkinerja tinggi: Mesin state handshake TLS 1.3 dan negosiasi cipher suite aman.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur rdma untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_tls_jabat_tangan_rdma = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_tls_jabat_tangan_rdma
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TlsJabatTanganRdma

publik fungsi utama(): Bulat {
    konst instansi = TlsJabatTanganRdma::inisialisasi()
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