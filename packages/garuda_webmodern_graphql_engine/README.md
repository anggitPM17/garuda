# garuda_webmodern_graphql_engine

> **Modul:** `GraphqlEngine` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Mesin eksekusi GraphQL dengan parsing query AST, resolver asinkron, dan dataloader batching.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pengembangan Website Modern.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_graphql_engine = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_graphql_engine
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GraphqlEngine

publik fungsi utama(): Bulat {
    konst instansi = GraphqlEngine::inisialisasi()
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