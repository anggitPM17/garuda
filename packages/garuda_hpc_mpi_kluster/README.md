# garuda_hpc_mpi_kluster

> **Modul:** `MpiKluster` • **Kategori:** Superkomputer & HPC Paralel • **Versi:** `0.1.0`

Protokol Message Passing Interface (MPI) nir-blokir untuk sinkronisasi antarsimpul superkomputer.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Superkomputer & HPC Paralel.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_hpc_mpi_kluster = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_hpc_mpi_kluster
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MpiKluster

publik fungsi utama(): Bulat {
    konst instansi = MpiKluster::inisialisasi()
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