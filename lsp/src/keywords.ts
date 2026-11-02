import { CompletionItem, CompletionItemKind } from 'vscode-languageserver/node';

export interface GarudaSymbol {
    label: string;
    kind: CompletionItemKind;
    documentation: string;
    detail?: string;
}

export const GARUDA_KEYWORDS: GarudaSymbol[] = [
    { label: 'modul', kind: CompletionItemKind.Keyword, documentation: 'Deklarasi modul Garuda (misal: modul NamaModul)' },
    { label: 'fungsi', kind: CompletionItemKind.Keyword, documentation: 'Deklarasi fungsi Garuda (misal: fungsi nama(Args) -> body rampung)' },
    { label: 'makro', kind: CompletionItemKind.Keyword, documentation: 'Deklarasi makro Garuda' },
    { label: 'rampung', kind: CompletionItemKind.Keyword, documentation: 'Penutup blok deklarasi/fungsi/pilihan di Garuda' },
    { label: 'yen', kind: CompletionItemKind.Keyword, documentation: 'Kata kunci percabangan (if) di Garuda' },
    { label: 'lain', kind: CompletionItemKind.Keyword, documentation: 'Kata kunci alternatif percabangan (else) di Garuda' },
    { label: 'nalika', kind: CompletionItemKind.Keyword, documentation: 'Kata kunci guard clause pada fungsi (misal: fungsi f(X) nalika X > 0 -> ...)' },
    { label: 'ketika', kind: CompletionItemKind.Keyword, documentation: 'Alias guard clause (when)' },
    { label: 'tinjau', kind: CompletionItemKind.Keyword, documentation: 'Kata kunci case/pattern matching (misal: tinjau Subjek ing ... rampung)' },
    { label: 'ing', kind: CompletionItemKind.Keyword, documentation: 'Pasangan tinjau (in)' },
    { label: 'cakra', kind: CompletionItemKind.Keyword, documentation: 'Spawns a new Actor process in GVM (misal: cakra fungsi() -> ... rampung)' },
    { label: 'tampa', kind: CompletionItemKind.Keyword, documentation: 'Menerima pesan actor (receive)' },
    { label: 'setelah', kind: CompletionItemKind.Keyword, documentation: 'Timeout pada receive actor (after)' },
    { label: 'diri', kind: CompletionItemKind.Keyword, documentation: 'Process ID dari actor saat ini (self)' },
    { label: 'kirim', kind: CompletionItemKind.Keyword, documentation: 'Kirim pesan ke actor PID (send)' },
    { label: 'coba', kind: CompletionItemKind.Keyword, documentation: 'Blok try/catch' },
    { label: 'tangkap', kind: CompletionItemKind.Keyword, documentation: 'Blok catch' },
    { label: 'impor', kind: CompletionItemKind.Keyword, documentation: 'Mengimpor pustaka standar (misal: impor "std/list.grd")' },
    { label: 'benar', kind: CompletionItemKind.Keyword, documentation: 'Literal Boolean true' },
    { label: 'salah', kind: CompletionItemKind.Keyword, documentation: 'Literal Boolean false' },
    { label: 'nil', kind: CompletionItemKind.Keyword, documentation: 'Literal Nil/Null' }
];

export const GARUDA_STDLIB: GarudaSymbol[] = [
    // List
    { label: 'List.panjang', kind: CompletionItemKind.Function, detail: 'List.panjang(Daftar) -> Integer', documentation: 'Mengembalikan jumlah elemen dalam list' },
    { label: 'List.kepala', kind: CompletionItemKind.Function, detail: 'List.kepala(Daftar) -> Any', documentation: 'Mengembalikan elemen pertama dalam list' },
    { label: 'List.ekor', kind: CompletionItemKind.Function, detail: 'List.ekor(Daftar) -> List', documentation: 'Mengembalikan sisa elemen list setelah elemen pertama' },
    { label: 'List.tambah', kind: CompletionItemKind.Function, detail: 'List.tambah(Daftar, Elemen) -> List', documentation: 'Menambahkan elemen ke akhir list' },
    { label: 'List.balik', kind: CompletionItemKind.Function, detail: 'List.balik(Daftar) -> List', documentation: 'Membalikkan urutan elemen dalam list' },
    { label: 'List.anggota', kind: CompletionItemKind.Function, detail: 'List.anggota(Daftar, X) -> Boolean', documentation: 'Memeriksa apakah X adalah anggota list' },

    // String
    { label: 'String.panjang', kind: CompletionItemKind.Function, detail: 'String.panjang(Teks) -> Integer', documentation: 'Mengembalikan panjang string' },
    { label: 'String.besar', kind: CompletionItemKind.Function, detail: 'String.besar(Teks) -> String', documentation: 'Mengubah string menjadi huruf besar' },
    { label: 'String.kecil', kind: CompletionItemKind.Function, detail: 'String.kecil(Teks) -> String', documentation: 'Mengubah string menjadi huruf kecil' },
    { label: 'String.potong', kind: CompletionItemKind.Function, detail: 'String.potong(Teks, Start, Len) -> String', documentation: 'Mengambil substring dari Mula sebanyak Panjang' },
    { label: 'String.pisah', kind: CompletionItemKind.Function, detail: 'String.pisah(Teks, Pembatas) -> List', documentation: 'Memecah string menjadi list string berdasarkan pembatas' },
    { label: 'String.mengandung', kind: CompletionItemKind.Function, detail: 'String.mengandung(Teks, Sub) -> Boolean', documentation: 'Memeriksa apakah string mengandung substring Sub' },
    { label: 'String.rapikan', kind: CompletionItemKind.Function, detail: 'String.rapikan(Teks) -> String', documentation: 'Menghapus whitespace di awal dan akhir string' },
    { label: 'String.ganti', kind: CompletionItemKind.Function, detail: 'String.ganti(Teks, From, To) -> String', documentation: 'Mengganti seluruh kemunculan From dengan To' },
    { label: 'String.cari', kind: CompletionItemKind.Function, detail: 'String.cari(Teks, Sub) -> Integer', documentation: 'Mencari indeks pertama Sub, atau -1 jika tidak ada' },
    { label: 'String.mulai_dengan', kind: CompletionItemKind.Function, detail: 'String.mulai_dengan(Teks, Prefix) -> Boolean', documentation: 'Memeriksa apakah string diawali dengan Prefix' },
    { label: 'String.akhir_dengan', kind: CompletionItemKind.Function, detail: 'String.akhir_dengan(Teks, Suffix) -> Boolean', documentation: 'Memeriksa apakah string diakhiri dengan Suffix' },
    { label: 'String.balik', kind: CompletionItemKind.Function, detail: 'String.balik(Teks) -> String', documentation: 'Membalikkan string' },
    { label: 'String.ulang', kind: CompletionItemKind.Function, detail: 'String.ulang(Teks, N) -> String', documentation: 'Mengulangi string N kali' },
    { label: 'String.ke_besar_awal', kind: CompletionItemKind.Function, detail: 'String.ke_besar_awal(Teks) -> String', documentation: 'Mengubah huruf pertama menjadi huruf besar' },

    // Map
    { label: 'Map.baru', kind: CompletionItemKind.Function, detail: 'Map.baru() -> Map', documentation: 'Membuat Map baru yang kosong' },
    { label: 'Map.taruh', kind: CompletionItemKind.Function, detail: 'Map.taruh(Map, Kunci, Nilai) -> Map', documentation: 'Menambahkan atau memperbarui entri pada Map' },
    { label: 'Map.ambil', kind: CompletionItemKind.Function, detail: 'Map.ambil(Map, Kunci) -> Any', documentation: 'Mengambil nilai berdasarkan Kunci dari Map' },
    { label: 'Map.hapus', kind: CompletionItemKind.Function, detail: 'Map.hapus(Map, Kunci) -> Map', documentation: 'Menghapus Kunci dari Map' },
    { label: 'Map.ukuran', kind: CompletionItemKind.Function, detail: 'Map.ukuran(Map) -> Integer', documentation: 'Mengembalikan jumlah entri pada Map' },
    { label: 'Map.mengandung_kunci', kind: CompletionItemKind.Function, detail: 'Map.mengandung_kunci(Map, Kunci) -> Boolean', documentation: 'Memeriksa apakah Kunci ada di Map' },
    { label: 'Map.kosong', kind: CompletionItemKind.Function, detail: 'Map.kosong(Map) -> Boolean', documentation: 'Memeriksa apakah Map kosong' },
    { label: 'Map.gabung', kind: CompletionItemKind.Function, detail: 'Map.gabung(M1, M2) -> Map', documentation: 'Menggabungkan dua Map' },

    // Tuple
    { label: 'Tuple.elemen', kind: CompletionItemKind.Function, detail: 'Tuple.elemen(Tuple, Index) -> Any', documentation: 'Mengambil elemen pada indeks (0-based) dari Tuple' },
    { label: 'Tuple.panjang', kind: CompletionItemKind.Function, detail: 'Tuple.panjang(Tuple) -> Integer', documentation: 'Mengembalikan jumlah elemen pada Tuple' },
    { label: 'Tuple.ke_list', kind: CompletionItemKind.Function, detail: 'Tuple.ke_list(Tuple) -> List', documentation: 'Mengonversi Tuple menjadi List' },
    { label: 'Tuple.gabung', kind: CompletionItemKind.Function, detail: 'Tuple.gabung(T1, T2) -> Tuple', documentation: 'Menggabungkan dua Tuple' },

    // Waktu
    { label: 'Waktu.sekarang', kind: CompletionItemKind.Function, detail: 'Waktu.sekarang() -> Integer', documentation: 'Mengembalikan Unix timestamp dalam milidetik' },
    { label: 'Waktu.tunggu', kind: CompletionItemKind.Function, detail: 'Waktu.tunggu(Ms) -> nil', documentation: 'Menunda eksekusi selama Ms milidetik' },
    { label: 'Waktu.durasi_mulai', kind: CompletionItemKind.Function, detail: 'Waktu.durasi_mulai() -> Integer', documentation: 'Mencatat timestamp awal untuk pengoperasian durasi' },
    { label: 'Waktu.durasi_selesai', kind: CompletionItemKind.Function, detail: 'Waktu.durasi_selesai(Start) -> Integer', documentation: 'Menghitung selisih milidetik dari timestamp Start' },

    // Matematika
    { label: 'Matematika.pangkat', kind: CompletionItemKind.Function, detail: 'Matematika.pangkat(Base, Exp) -> Integer', documentation: 'Menghitung Base^Exp' },
    { label: 'Matematika.abs', kind: CompletionItemKind.Function, detail: 'Matematika.abs(X) -> Integer', documentation: 'Mengembalikan nilai mutlak/absolut dari X' },
    { label: 'Matematika.min', kind: CompletionItemKind.Function, detail: 'Matematika.min(A, B) -> Integer', documentation: 'Mengembalikan nilai terkecil antara A dan B' },
    { label: 'Matematika.max', kind: CompletionItemKind.Function, detail: 'Matematika.max(A, B) -> Integer', documentation: 'Mengembalikan nilai terbesar antara A dan B' },
    { label: 'Matematika.akar', kind: CompletionItemKind.Function, detail: 'Matematika.akar(X) -> Integer', documentation: 'Menghitung akar kuadrat integer (floor)' },

    // Http
    { label: 'Http.ambil', kind: CompletionItemKind.Function, detail: 'Http.ambil(Url) -> String', documentation: 'Mengirimkan HTTP GET request dan mengembalikan response body' },
    { label: 'Http.kirim_json', kind: CompletionItemKind.Function, detail: 'Http.kirim_json(Url, JsonData) -> String', documentation: 'Mengirimkan HTTP POST JSON request' },

    // Web
    { label: 'Web.daftar_route', kind: CompletionItemKind.Function, detail: 'Web.daftar_route(Method, Path, Handler) -> :ok', documentation: 'Mendaftarkan rute web dinamis' },
    { label: 'Web.get', kind: CompletionItemKind.Function, detail: 'Web.get(Path, Handler) -> :ok', documentation: 'Shorthand pendaftaran rute HTTP GET' },
    { label: 'Web.post', kind: CompletionItemKind.Function, detail: 'Web.post(Path, Handler) -> :ok', documentation: 'Shorthand pendaftaran rute HTTP POST' },
    { label: 'Web.jalankan', kind: CompletionItemKind.Function, detail: 'Web.jalankan(Port) -> :ok', documentation: 'Jalankan HTTP Web Server pada Port' }
];
