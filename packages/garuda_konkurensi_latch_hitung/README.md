# garuda_konkurensi_latch_hitung

> **Modul:** `LatchHitung` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Pencacah mundur sinkronisasi menunggu sejumlah operasi asinkron selesai.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Konkurensi & Aktor.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_latch_hitung = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_latch_hitung
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor LatchHitung

publik fungsi utama(): Bulat {
    konst instansi = LatchHitung::inisialisasi()
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