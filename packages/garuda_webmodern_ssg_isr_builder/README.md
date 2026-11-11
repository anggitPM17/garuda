# garuda_webmodern_ssg_isr_builder

> **Modul:** `SsgIsrBuilder` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Pembangun situs statis (SSG) dan Incremental Static Regeneration (ISR) berbasis cache berwaktu.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pengembangan Website Modern.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_ssg_isr_builder = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_ssg_isr_builder
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SsgIsrBuilder

publik fungsi utama(): Bulat {
    konst instansi = SsgIsrBuilder::inisialisasi()
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