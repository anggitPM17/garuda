# garuda_http_klien_v4

> **Modul:** `HttpKlienV4` • **Kategori:** Jaringan & Web

Klien HTTP/1.1 dan HTTP/2 asinkron dengan pooling koneksi otomatis.

## Alasan Pengayaan Ekosistem Garuda
Mendukung integrasi API eksternal dan microservices.

## Instalasi via GPM
```bash
gpm pasang garuda_http_klien_v4
```

## Contoh Penggunaan
```garuda
impor HttpKlienV4

publik fungsi utama(): Bulat {
    konst cfg = HttpKlienV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.