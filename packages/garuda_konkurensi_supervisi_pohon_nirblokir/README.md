# garuda_konkurensi_supervisi_pohon_nirblokir

> **Modul:** `SupervisiPohonNirblokir` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Pohon supervisi fault-tolerant dengan strategi restart one-for-one dan one-for-all.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_supervisi_pohon_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_supervisi_pohon_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SupervisiPohonNirblokir

publik fungsi utama(): Bulat {
    konst instansi = SupervisiPohonNirblokir::inisialisasi()
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