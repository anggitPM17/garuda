# garuda_telemetri_pencatat_terstruktur

> **Modul:** `PencatatTerstruktur` • **Kategori:** Telemetri & Observabilitas • **Versi:** `0.1.0`

Logger terstruktur format JSON dengan tingkat level galat, info, dan jejak.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Telemetri & Observabilitas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_telemetri_pencatat_terstruktur = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_telemetri_pencatat_terstruktur
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor PencatatTerstruktur

publik fungsi utama(): Bulat {
    konst instansi = PencatatTerstruktur::inisialisasi()
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