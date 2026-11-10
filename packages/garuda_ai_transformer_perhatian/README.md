# garuda_ai_transformer_perhatian

> **Modul:** `TransformerPerhatian` • **Kategori:** Kecerdasan Buatan & Neural • **Versi:** `0.1.0`

Mekanisme multi-head self-attention dan feed-forward transformer untuk inferensi LLM.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kecerdasan Buatan & Neural.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_ai_transformer_perhatian = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_ai_transformer_perhatian
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TransformerPerhatian

publik fungsi utama(): Bulat {
    konst instansi = TransformerPerhatian::inisialisasi()
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