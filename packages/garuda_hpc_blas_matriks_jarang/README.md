# garuda_hpc_blas_matriks_jarang

> **Modul:** `BlasMatriksJarang` • **Kategori:** Superkomputer & HPC Paralel • **Versi:** `0.1.0`

Perkalian matriks jarang sparse BLAS format CSR/CSC terakselerasi CPU multi-core.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Superkomputer & HPC Paralel.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_hpc_blas_matriks_jarang = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_hpc_blas_matriks_jarang
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BlasMatriksJarang

publik fungsi utama(): Bulat {
    konst instansi = BlasMatriksJarang::inisialisasi()
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