#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>

using namespace garuda::jalur_langit;

#define CHECK(cond, msg) \
    if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "JALUR LANGIT ENGINE TESTS\n";
    std::cout << "========================================\n\n";

    // 1. Inisialisasi Database
    std::cout << "[1] Menginisialisasi Jalur Langit Database...\n";
    std::string test_data_dir = "./test_jalur_langit_data";
    std::filesystem::remove_all(test_data_dir); // Bersihkan jika ada sisa

    {
        JalurLangitUltimate db(test_data_dir);
        std::cout << "PASS: Database siap.\n\n";

        // 2. Membuat Tabel
        std::cout << "[2] Membuat tabel 'users'...\n";
        std::vector<ColumnDef> columns = {
            {"id", "INT", true},
            {"nama", "STRING", false},
            {"umur", "INT", false}
        };
        db.create_table("users", columns);
        std::cout << "PASS: Tabel 'users' berhasil dibuat.\n\n";

        // 3. Eksekusi Query (Simulasi)
        std::cout << "[3] Menjalankan query INSERT...\n";
        auto res = db.execute("INSERT INTO users VALUES (1, 'Budi', 25)");
        CHECK(res.success == true, "Query INSERT harus sukses");
        CHECK(res.affected_rows == 1, "Harus ada 1 baris yang terpengaruh");
        std::cout << "PASS: Query INSERT berhasil.\n\n";

        // 4. Membuat Index
        std::cout << "[4] Membuat index pada kolom 'nama'...\n";
        db.create_index("users", "nama");
        std::cout << "PASS: Index berhasil dibuat.\n\n";
    }

    // 5. Verifikasi WAL
    std::cout << "[5] Memverifikasi file WAL...\n";
    std::string wal_path = test_data_dir + "/wal.log";
    CHECK(std::filesystem::exists(wal_path), "File wal.log harus terbentuk");

    {
        std::ifstream wal_file(wal_path);
        std::string line;
        bool found_insert = false;
        while (std::getline(wal_file, line)) {
            if (line.find("INSERT") != std::string::npos) {
                found_insert = true;
                break;
            }
        }
        CHECK(found_insert, "File WAL harus berisi catatan operasi INSERT");
    }
    std::cout << "PASS: File WAL terverifikasi berisi catatan INSERT.\n\n";

    // Bersihkan direktori tes
    std::error_code ec;
    std::filesystem::remove_all(test_data_dir, ec);

    std::cout << "========================================\n";
    std::cout << "JALUR LANGIT FOUNDATION IS READY!\n";
    std::cout << "========================================\n";
    return 0;
}
