# garuda_grafika_matriks_komputasi_cepat

> **Modul:** `MatriksKomputasiCepat` • **Kategori:** Matematika & Grafika • **Versi:** `0.1.0`

Varian Cepat berkinerja tinggi: Operasi aljabar matriks NxN, eliminasi Gauss-Jordan, dan dekomposisi LU.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur cepat untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_grafika_matriks_komputasi_cepat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_grafika_matriks_komputasi_cepat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MatriksKomputasiCepat

publik fungsi utama(): Bulat {
    konst instansi = MatriksKomputasiCepat::inisialisasi()
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