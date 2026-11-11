# garuda_sistem_pemantau_direktori

> **Modul:** `PemantauDirektori` • **Kategori:** Sistem & OS • **Versi:** `0.1.0`

Pendeteksi perubahan berkas dan direktori sistem berkas secara asinkron.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Sistem & OS.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sistem_pemantau_direktori = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sistem_pemantau_direktori
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor PemantauDirektori

publik fungsi utama(): Bulat {
    konst instansi = PemantauDirektori::inisialisasi()
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