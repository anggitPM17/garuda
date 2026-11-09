# garuda_sains_probabilitas_distribusi_aman

> **Modul:** `ProbabilitasDistribusiAman` • **Kategori:** Statistik & Sains • **Versi:** `0.1.0`

Varian Aman berkinerja tinggi: Fungsi kepadatan probabilitas distribusi Normal, Poisson, dan Binomial.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur aman untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sains_probabilitas_distribusi_aman = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sains_probabilitas_distribusi_aman
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ProbabilitasDistribusiAman

publik fungsi utama(): Bulat {
    konst instansi = ProbabilitasDistribusiAman::inisialisasi()
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