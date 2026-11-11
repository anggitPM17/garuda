# garuda_bioinformatika_rna_folding

> **Modul:** `BioinformatikaRnaFolding` • **Kategori:** Bioinformatika & Genomika • **Versi:** `0.1.0`

Prediksi struktur sekunder pelipatan RNA berbasis energi bebas minimum (Minimum Free Energy - Nussinov & Zuker dynamic programming) dengan notasi Dot-Bracket.

## Arsitektur & Tujuan
Pemodelan biologi molekuler, interaksi mRNA vaksin, dan analisis ribosom sel.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_bioinformatika_rna_folding = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_bioinformatika_rna_folding
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BioinformatikaRnaFolding

publik fungsi utama(): Bulat {
    konst instansi = BioinformatikaRnaFolding::inisialisasi()
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