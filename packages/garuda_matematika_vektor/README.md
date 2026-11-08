# garuda_matematika_vektor

> **Modul:** `MatematikaVektor` • **Kategori:** Matematika & Grafika • **Versi:** `0.1.0`

Aljabar linier komputasi Vektor 2D/3D dan Matriks 4x4 dengan operasi perkalian silang, dot product, determinan, dan inversi.

## Arsitektur & Tujuan
Pondasi simulasi fisika, transformasi grafika 3D, dan game engine di runtime GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_matematika_vektor = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_matematika_vektor
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MatematikaVektor

publik fungsi utama(): Bulat {
    konst instansi = MatematikaVektor::inisialisasi()
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