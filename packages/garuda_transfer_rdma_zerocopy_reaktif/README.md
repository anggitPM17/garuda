# garuda_transfer_rdma_zerocopy_reaktif

> **Modul:** `RdmaZeroCopyReaktif` • **Kategori:** Transfer Data Kecepatan Ekstrem • **Versi:** `0.1.0`

Varian Reaktif berkinerja tinggi: Remote Direct Memory Access (RDMA) bypass kernel tanpa penyalinan memori (zero-copy).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur reaktif untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_transfer_rdma_zerocopy_reaktif = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_transfer_rdma_zerocopy_reaktif
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RdmaZeroCopyReaktif

publik fungsi utama(): Bulat {
    konst instansi = RdmaZeroCopyReaktif::inisialisasi()
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