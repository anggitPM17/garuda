# garuda_sistem_sinyal_proses_nirblokir

> **Modul:** `SinyalProsesNirblokir` • **Kategori:** Sistem & OS • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Penanganan sinyal POSIX seperti SIGINT dan SIGTERM untuk graceful shutdown aplikasi.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sistem_sinyal_proses_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sistem_sinyal_proses_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SinyalProsesNirblokir

publik fungsi utama(): Bulat {
    konst instansi = SinyalProsesNirblokir::inisialisasi()
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