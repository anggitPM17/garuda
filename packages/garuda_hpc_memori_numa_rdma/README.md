# garuda_hpc_memori_numa_rdma

> **Modul:** `MemoriNumaRdma` • **Kategori:** Superkomputer & HPC Paralel • **Versi:** `0.1.0`

Varian Rdma berkinerja tinggi: Pengalokasi halaman memori sadar topologi NUMA (Non-Uniform Memory Access).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur rdma untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_hpc_memori_numa_rdma = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_hpc_memori_numa_rdma
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MemoriNumaRdma

publik fungsi utama(): Bulat {
    konst instansi = MemoriNumaRdma::inisialisasi()
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