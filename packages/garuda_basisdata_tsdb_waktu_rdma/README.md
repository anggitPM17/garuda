# garuda_basisdata_tsdb_waktu_rdma

> **Modul:** `TsdbWaktuRdma` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Varian Rdma berkinerja tinggi: Database deret waktu dengan kompresi Gorilla Delta-of-Delta untuk telemetri.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur rdma untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_tsdb_waktu_rdma = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_tsdb_waktu_rdma
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TsdbWaktuRdma

publik fungsi utama(): Bulat {
    konst instansi = TsdbWaktuRdma::inisialisasi()
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