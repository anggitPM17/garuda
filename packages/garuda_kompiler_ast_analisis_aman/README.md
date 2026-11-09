# garuda_kompiler_ast_analisis_aman

> **Modul:** `AstAnalisisAman` • **Kategori:** Kompiler & Bahasa • **Versi:** `0.1.0`

Varian Aman berkinerja tinggi: Pengunjung pohon sintaksis abstrak (AST Visitor) untuk linting dan transformasi kode.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur aman untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompiler_ast_analisis_aman = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompiler_ast_analisis_aman
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AstAnalisisAman

publik fungsi utama(): Bulat {
    konst instansi = AstAnalisisAman::inisialisasi()
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