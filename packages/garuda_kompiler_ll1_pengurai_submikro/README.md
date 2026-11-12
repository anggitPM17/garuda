# garuda_kompiler_ll1_pengurai_submikro

> **Modul:** `Ll1PenguraiSubmikro` • **Kategori:** Kompiler & Bahasa • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Kerangka parser rekursif menurun LL(1) dengan tabel first and follow.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompiler_ll1_pengurai_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompiler_ll1_pengurai_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Ll1PenguraiSubmikro

publik fungsi utama(): Bulat {
    konst instansi = Ll1PenguraiSubmikro::inisialisasi()
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