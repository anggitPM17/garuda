# garuda_bioinformatika_dna_alignment

> **Modul:** `BioinformatikaDnaAlignment` • **Kategori:** Bioinformatika & Genomika • **Versi:** `0.1.0`

Algoritma penyejajaran urutan sekuens DNA/Protein global (Needleman-Wunsch) dan lokal (Smith-Waterman) dengan penalti celah afin (affine gap penalty).

## Arsitektur & Tujuan
Penelitian mutasi genetik, rekonstruksi filogenetik, dan pencocokan homologi rantai nukleotida genomik.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_bioinformatika_dna_alignment = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_bioinformatika_dna_alignment
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BioinformatikaDnaAlignment

publik fungsi utama(): Bulat {
    konst instansi = BioinformatikaDnaAlignment::inisialisasi()
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