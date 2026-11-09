# garuda_kompiler_ast_analisis

> **Modul:** `AstAnalisis` • **Kategori:** Kompiler & Bahasa • **Versi:** `0.1.0`

Pengunjung pohon sintaksis abstrak (AST Visitor) untuk linting dan transformasi kode.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kompiler & Bahasa.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompiler_ast_analisis = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompiler_ast_analisis
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AstAnalisis

publik fungsi utama(): Bulat {
    konst instansi = AstAnalisis::inisialisasi()
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