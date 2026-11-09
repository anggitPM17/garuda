# garuda_media_wav_audio

> **Modul:** `WavAudio` • **Kategori:** Audio & Sinyal • **Versi:** `0.1.0`

Parser dan enkoder berkas audio RIFF WAV PCM 16/24/32-bit integer dan float.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Audio & Sinyal.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_media_wav_audio = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_media_wav_audio
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WavAudio

publik fungsi utama(): Bulat {
    konst instansi = WavAudio::inisialisasi()
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