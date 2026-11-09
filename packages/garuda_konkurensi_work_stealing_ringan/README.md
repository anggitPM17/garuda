# garuda_konkurensi_work_stealing_ringan

> **Modul:** `WorkStealingRingan` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Varian Ringan berkinerja tinggi: Pool thread dengan mekanisme pencurian tugas untuk pembagian beban kerja optimal.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur ringan untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_work_stealing_ringan = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_work_stealing_ringan
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WorkStealingRingan

publik fungsi utama(): Bulat {
    konst instansi = WorkStealingRingan::inisialisasi()
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