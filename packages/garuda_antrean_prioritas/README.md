# garuda_antrean_prioritas

> **Modul:** `AntreanPrioritas` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Binary Min/Max Heap thread-safe berurutan stabil dengan dukungan penyeimbangan naik/turun otomatis.

## Arsitektur & Tujuan
Pondasi scheduler antrean tugas konkuren runtime GVM dan pemeringkatan prioritas aktor.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_antrean_prioritas = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_antrean_prioritas
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AntreanPrioritas

publik fungsi utama(): Bulat {
    konst instansi = AntreanPrioritas::inisialisasi()
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