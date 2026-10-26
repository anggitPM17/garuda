#pragma once
// Garuda Language Compiler: Source Map & Diagnostic Error Reporter (`source_map.hpp`)
// Mengimplementasikan Pemetaan Bytecode Index ke Baris/Kolom Kode Sumber & Formatting Error Snippet dengan Caret (^).
#include "../beam/gvm/interpreter.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

struct SourceLocation {
    std::size_t line = 1;
    std::size_t col = 1;
    std::string file_name = "main.grd";
};

class SourceMap {
public:
    SourceMap() = default;

    void add_mapping(std::size_t instruction_ip, std::size_t line, std::size_t col, std::string file_name = "main.grd") {
        mappings_[instruction_ip] = SourceLocation{line, col, std::move(file_name)};
    }

    SourceLocation get_location(std::size_t instruction_ip) const {
        auto it = mappings_.find(instruction_ip);
        if (it != mappings_.end()) return it->second;
        return SourceLocation{1, 1, "main.grd"};
    }

    static std::string format_diagnostic(
        const std::string& source_code,
        std::size_t line,
        std::size_t col,
        const std::string& message,
        const std::string& file_name = "main.grd"
    ) {
        std::ostringstream ss;
        ss << "❌ " << file_name << ":" << line << ":" << col << " - " << message << "\n";

        // Extract error line from source code
        std::istringstream src_ss(source_code);
        std::string src_line;
        std::size_t current_line = 1;

        while (std::getline(src_ss, src_line)) {
            if (current_line == line) {
                if (!src_line.empty() && src_line.back() == '\r') src_line.pop_back();

                // Line number header
                std::string line_num_str = std::to_string(line);
                std::string pad(4 > line_num_str.length() ? 4 - line_num_str.length() : 0, ' ');
                ss << pad << line_num_str << " | " << src_line << "\n";

                // Caret alignment
                std::string caret_pad(4 + 3 + (col > 0 ? col - 1 : 0), ' ');
                ss << caret_pad << "^\n";
                break;
            }
            current_line++;
        }

        return ss.str();
    }

private:
    std::map<std::size_t, SourceLocation> mappings_;
};

} // namespace garuda::compiler
