# garuda_jaringan_grpc_transport

> **Modul:** `GrpcTransport` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Transportasi framing gRPC melalui HTTP/2 dengan kompresi protobuf.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Jaringan & Protokol.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_grpc_transport = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_grpc_transport
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GrpcTransport

publik fungsi utama(): Bulat {
    konst instansi = GrpcTransport::inisialisasi()
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