# garuda_format_json_serializer

> **Modul:** `JsonSerializer` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Serializer dan parser JSON streaming nir-alokasi berbasis state machine.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Parsing & Format Data.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_json_serializer = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_json_serializer
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor JsonSerializer

publik fungsi utama(): Bulat {
    konst instansi = JsonSerializer::inisialisasi()
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