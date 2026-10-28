#include "../include/garuda/jalur_langit/json_serializer.hpp"
#include <iostream>
#include <cstdlib>

using namespace garuda::jalur_langit;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "=== JSON SERIALIZER TESTS ===\n";

    Row row;
    row.columns["id"] = std::int64_t(1);
    row.columns["nama"] = std::string("Budi");
    row.columns["aktif"] = true;
    row.columns["skor"] = 95.5;

    std::string json = JsonSerializer::serialize_row(row);
    std::cout << "Row JSON: " << json << "\n";

    CHECK(json.find("\"id\":1") != std::string::npos, "JSON harus berisi id:1");
    CHECK(json.find("\"nama\":\"Budi\"") != std::string::npos, "JSON harus berisi nama");
    CHECK(json.find("\"aktif\":true") != std::string::npos, "JSON harus berisi aktif:true");
    CHECK(json.find("\"skor\":95.5") != std::string::npos, "JSON harus berisi skor:95.5");
    std::cout << "PASS: Row serialization bekerja.\n";

    QueryResult res;
    res.success = true;
    res.rows.push_back(row);
    std::string result_json = JsonSerializer::serialize_result(res);
    std::cout << "Result JSON: " << result_json << "\n";
    CHECK(result_json.find("\"success\":true") != std::string::npos, "Result harus berisi success:true");
    std::cout << "PASS: Result serialization bekerja.\n";

    std::cout << "JSON SERIALIZER TESTS PASSED!\n";
    return 0;
}
