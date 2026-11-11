# garuda_kuantum_circuit_engine

> **Modul:** `KuantumCircuitEngine` • **Kategori:** Komputasi Kuantum • **Versi:** `0.1.0`

Mesin eksekusi Quantum Circuit multi-qubit dengan penjadwalan gerbang berwaktu, dekoherensi noise, dan visualisasi diagram sirkuit ASCII.

## Arsitektur & Tujuan
Simulator sirkuit kuantum virtual terdistribusi untuk pengujian algoritma kuantum sebelum dikirim ke QPU fisik.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kuantum_circuit_engine = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kuantum_circuit_engine
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KuantumCircuitEngine

publik fungsi utama(): Bulat {
    konst instansi = KuantumCircuitEngine::inisialisasi()
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