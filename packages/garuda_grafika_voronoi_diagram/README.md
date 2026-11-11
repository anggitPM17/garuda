# garuda_grafika_voronoi_diagram

> **Modul:** `VoronoiDiagram` • **Kategori:** Matematika & Grafika • **Versi:** `0.1.0`

Generasi sel Voronoi dan triangulasi Delaunay untuk partisi ruang 2D.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Matematika & Grafika.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_grafika_voronoi_diagram = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_grafika_voronoi_diagram
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor VoronoiDiagram

publik fungsi utama(): Bulat {
    konst instansi = VoronoiDiagram::inisialisasi()
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