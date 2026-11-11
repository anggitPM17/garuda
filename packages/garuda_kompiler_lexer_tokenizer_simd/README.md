# garuda_kompiler_lexer_tokenizer_simd

> **Modul:** `LexerTokenizerSimd` • **Kategori:** Kompiler & Bahasa • **Versi:** `0.1.0`

Varian Simd berkinerja tinggi: Pemecah aliran karakter sumber menjadi token dengan pelacakan posisi kolom/baris.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur simd untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompiler_lexer_tokenizer_simd = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompiler_lexer_tokenizer_simd
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor LexerTokenizerSimd

publik fungsi utama(): Bulat {
    konst instansi = LexerTokenizerSimd::inisialisasi()
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