# garuda_antrean_prioritas_v6

> **Modul:** `AntreanPrioritasV6` • **Kategori:** Struktur Data & Algoritma

Binary Min/Max Heap thread-safe untuk penjadwalan antrean pesan aktor.

## Alasan Pengayaan Ekosistem Garuda
Pondasi scheduler tugas konkuren di runtime Garuda.

## Instalasi via GPM
```bash
gpm pasang garuda_antrean_prioritas_v6
```

## Contoh Penggunaan
```garuda
impor AntreanPrioritasV6

publik fungsi utama(): Bulat {
    konst cfg = AntreanPrioritasV6::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.