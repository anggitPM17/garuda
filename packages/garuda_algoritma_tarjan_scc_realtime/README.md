# garuda_algoritma_tarjan_scc_realtime

> **Modul:** `TarjanSccRealtime` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Algoritma pencarian komponen terhubung kuat dalam graf berarah siklik.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_tarjan_scc_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_tarjan_scc_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TarjanSccRealtime

publik fungsi utama(): Bulat {
    konst instansi = TarjanSccRealtime::inisialisasi()
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