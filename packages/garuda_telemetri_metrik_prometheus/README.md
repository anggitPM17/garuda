# garuda_telemetri_metrik_prometheus

> **Modul:** `MetrikPrometheus` • **Kategori:** Telemetri & Observabilitas • **Versi:** `0.1.0`

Kolektor metrik format ekspos Prometheus (Counter, Gauge, Histogram, Summary).

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Telemetri & Observabilitas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_telemetri_metrik_prometheus = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_telemetri_metrik_prometheus
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MetrikPrometheus

publik fungsi utama(): Bulat {
    konst instansi = MetrikPrometheus::inisialisasi()
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