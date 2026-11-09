# garuda_konkurensi_aktor_genserver

> **Modul:** `AktorGenserver` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Pola perilaku Generic Server berbasis aktor dengan panggilan sinkron/asinkron.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Konkurensi & Aktor.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_aktor_genserver = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_aktor_genserver
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AktorGenserver

publik fungsi utama(): Bulat {
    konst instansi = AktorGenserver::inisialisasi()
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