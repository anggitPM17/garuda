# garuda_konkurensi_supervisi_pohon

> **Modul:** `SupervisiPohon` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Pohon supervisi fault-tolerant dengan strategi restart one-for-one dan one-for-all.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Konkurensi & Aktor.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_supervisi_pohon = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_supervisi_pohon
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SupervisiPohon

publik fungsi utama(): Bulat {
    konst instansi = SupervisiPohon::inisialisasi()
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