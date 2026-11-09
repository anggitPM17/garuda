# garuda_konkurensi_saluran_csp

> **Modul:** `SaluranCsp` • **Kategori:** Konkurensi & Aktor • **Versi:** `0.1.0`

Saluran komunikasi berkapasitas dan nir-kapasitas ala Communicating Sequential Processes.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Konkurensi & Aktor.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_konkurensi_saluran_csp = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_konkurensi_saluran_csp
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SaluranCsp

publik fungsi utama(): Bulat {
    konst instansi = SaluranCsp::inisialisasi()
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