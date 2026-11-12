# garuda_jaringan_tls_jabat_tangan_submikro

> **Modul:** `TlsJabatTanganSubmikro` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Mesin state handshake TLS 1.3 dan negosiasi cipher suite aman.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_tls_jabat_tangan_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_tls_jabat_tangan_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TlsJabatTanganSubmikro

publik fungsi utama(): Bulat {
    konst instansi = TlsJabatTanganSubmikro::inisialisasi()
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