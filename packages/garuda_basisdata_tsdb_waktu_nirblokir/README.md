# garuda_basisdata_tsdb_waktu_nirblokir

> **Modul:** `TsdbWaktuNirblokir` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Database deret waktu dengan kompresi Gorilla Delta-of-Delta untuk telemetri.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_tsdb_waktu_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_tsdb_waktu_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TsdbWaktuNirblokir

publik fungsi utama(): Bulat {
    konst instansi = TsdbWaktuNirblokir::inisialisasi()
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