# 🏛️ Tata Kelola Proyek Garuda (Governance)

Dokumen ini menjelaskan struktur kepemimpinan, proses pengambilan keputusan, dan alur menjadi kontributor tetap pada proyek open-source **Garuda**.

---

## 👑 1. Model Keputusan (BDFL)

Proyek Garuda menggunakan model kepemimpinan **BDFL (Benevolent Dictator for Life)**:
- **BDFL / Pemilik Proyek**: Pemegang keputusan akhir terkait arsitektur inti, filosofi desain, dan roadmap proyek.
- **Tanggung Jawab Pemilik Proyek**: Menjaga kualitas arsitektur, determinisme build, keamanan, dan memastikan setiap perubahan selaras dengan visi jangka panjang Garuda.

---

## 🌟 2. Jalur Menjadi Kontributor Tetap (Core Contributor)

Setiap pengembang di komunitas memiliki kesempatan untuk menjadi **Core Contributor** dengan hak akses review dan maintainer melalui tahapan berikut:

1. **Contributor**: Seseorang yang mengirimkan minimal 1 Pull Request berkualitas yang berhasil di-merge (perbaikan bug, dokumentasi, atau unit test).
2. **Active Contributor**: Seseorang yang secara konsisten berkontribusi minimal 5 Pull Request berkualitas, berpartisipasi aktif dalam review kode, dan membantu menjawab isu komunitas.
3. **Core Maintainer / Reviewer**: Diberikan oleh BDFL kepada Active Contributor yang terbukti memahami arsitektur inti GVM dan menunjukkan komitmen jangka panjang.

---

## 📐 3. Pengambilan Keputusan Teknis Besar (RFC)

Untuk perubahan arsitektur besar (seperti perubahan spesifikasi bytecode, perombakan GC, atau penambahan protokol jaringan baru):

1. **Buat Issue RFC (Request for Comments)**: Jelaskan latar belakang masalah, usulan solusi teknis, dan alternatif pendekatan.
2. **Diskusi Komunitas**: Memberikan waktu bagi anggota komunitas untuk memberikan masukan selama minimal 7 hari.
3. **Persetujuan Akhir**: BDFL mengevaluasi konsensus dan memberikan keputusan final sebelum implementasi dimulai.
