#pragma once
// Garuda Standard Library: IO Module (`io.hpp`)
// Mengimplementasikan I/O konsol standar (cetak, cetak_galat, baca_baris).
#include "../core/term.hpp"

#include <iostream>
#include <string>

namespace garuda::beam::stdlib {

class IO {
public:
    static void cetak(const std::string& s) {
        std::cout << s << std::endl;
    }

    static void cetak_term(const Term& t) {
        std::cout << to_string(t) << std::endl;
    }

    static void cetak_galat(const std::string& s) {
        std::cerr << "⚠️ " << s << std::endl;
    }

    static std::string baca_baris() {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }
};

} // namespace garuda::beam::stdlib
