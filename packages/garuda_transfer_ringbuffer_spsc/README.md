# garuda_transfer_ringbuffer_spsc

> **Modul:** `RingbufferSpsc` • **Kategori:** Transfer Data Kecepatan Ekstrem • **Versi:** `0.1.0`

Buffer melingkar lock-free Single-Producer Single-Consumer berlatensi sub-mikrodetik.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Transfer Data Kecepatan Ekstrem.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_transfer_ringbuffer_spsc = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_transfer_ringbuffer_spsc
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RingbufferSpsc

publik fungsi utama(): Bulat {
    konst instansi = RingbufferSpsc::inisialisasi()
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