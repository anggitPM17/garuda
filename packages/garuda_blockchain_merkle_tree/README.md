# garuda_blockchain_merkle_tree

> **Modul:** `BlockchainMerkleTree` • **Kategori:** Blockchain & Web3 • **Versi:** `0.1.0`

Pohon Merkle kriptografis biner untuk verifikasi keabsahan transaksi, audit bukti keanggotaan (Merkle Proof), dan state root rollup.

## Arsitektur & Tujuan
Verifikasi integritas struktur data terdistribusi nir-kepercayaan (trustless) dan light client verification.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_blockchain_merkle_tree = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_blockchain_merkle_tree
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BlockchainMerkleTree

publik fungsi utama(): Bulat {
    konst instansi = BlockchainMerkleTree::inisialisasi()
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