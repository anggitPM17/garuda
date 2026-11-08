# garuda_antrean_prioritas_v4

> **Modul:** `AntreanPrioritasV4` • **Kategori:** Struktur Data & Algoritma

Binary Min/Max Heap thread-safe untuk penjadwalan antrean pesan aktor.

## Alasan Pengayaan Ekosistem Garuda
Pondasi scheduler tugas konkuren di runtime Garuda.

## Instalasi via GPM
```bash
gpm pasang garuda_antrean_prioritas_v4
```

## Contoh Penggunaan
```garuda
impor AntreanPrioritasV4

publik fungsi utama(): Bulat {
    konst cfg = AntreanPrioritasV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.