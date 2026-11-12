# garuda_transfer_rdma_zerocopy_realtime

> **Modul:** `RdmaZeroCopyRealtime` • **Kategori:** Transfer Data Kecepatan Ekstrem • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Remote Direct Memory Access (RDMA) bypass kernel tanpa penyalinan memori (zero-copy).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_transfer_rdma_zerocopy_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_transfer_rdma_zerocopy_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RdmaZeroCopyRealtime

publik fungsi utama(): Bulat {
    konst instansi = RdmaZeroCopyRealtime::inisialisasi()
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