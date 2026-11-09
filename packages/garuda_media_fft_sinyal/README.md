# garuda_media_fft_sinyal

> **Modul:** `FftSinyal` • **Kategori:** Audio & Sinyal • **Versi:** `0.1.0`

Fast Fourier Transform algoritma Cooley-Tukey untuk analisis spektrum frekuensi.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Audio & Sinyal.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_media_fft_sinyal = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_media_fft_sinyal
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor FftSinyal

publik fungsi utama(): Bulat {
    konst instansi = FftSinyal::inisialisasi()
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