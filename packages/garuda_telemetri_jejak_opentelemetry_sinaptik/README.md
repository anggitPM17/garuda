# garuda_telemetri_jejak_opentelemetry_sinaptik

> **Modul:** `JejakOpentelemetrySinaptik` • **Kategori:** Telemetri & Observabilitas • **Versi:** `0.1.0`

Varian Sinaptik berkinerja tinggi: Pelacakan jejak terdistribusi (Distributed Tracing) kompatibel W3C TraceContext.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur sinaptik untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_telemetri_jejak_opentelemetry_sinaptik = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_telemetri_jejak_opentelemetry_sinaptik
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor JejakOpentelemetrySinaptik

publik fungsi utama(): Bulat {
    konst instansi = JejakOpentelemetrySinaptik::inisialisasi()
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