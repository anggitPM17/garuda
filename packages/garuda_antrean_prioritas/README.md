# garuda_antrean_prioritas

> **Modul:** `AntreanPrioritas` • **Kategori:** Struktur Data & Algoritma

Binary Min/Max Heap thread-safe untuk penjadwalan antrean pesan aktor.

## Alasan Pengayaan Ekosistem Garuda
Pondasi scheduler tugas konkuren di runtime Garuda.

## Instalasi via GPM
```bash
gpm pasang garuda_antrean_prioritas
```

## Contoh Penggunaan
```garuda
impor AntreanPrioritas

publik fungsi utama(): Bulat {
    konst cfg = AntreanPrioritas::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.