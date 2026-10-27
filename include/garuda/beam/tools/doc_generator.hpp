#pragma once
// Garuda Ecosystem Tools: Automated API Documentation Generator (`doc_generator.hpp`)
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

namespace fs = std::filesystem;

struct DocItem {
    std::string func_name;
    std::string signature;
    std::string doc_comment;
};

struct DocPage {
    std::string file_name;
    std::vector<DocItem> items;
};

class DocGenerator {
public:
    static DocPage parse_file(const std::string& path) {
        DocPage page;
        fs::path p(path);
        page.file_name = p.filename().string();

        std::ifstream ifs(path);
        if (!ifs.is_open()) return page;

        std::string line;
        std::string accumulated_doc;

        while (std::getline(ifs, line)) {
            std::string trimmed = trim(line);

            if (trimmed.rfind("///", 0) == 0) {
                // Doc comment line
                std::string comment_text = trimmed.substr(3);
                if (!comment_text.empty() && comment_text.front() == ' ') {
                    comment_text.erase(0, 1);
                }
                if (!accumulated_doc.empty()) accumulated_doc += "\n";
                accumulated_doc += comment_text;
            } else if (trimmed.rfind("fun ", 0) == 0 || trimmed.rfind("fungsi ", 0) == 0) {
                // Function declaration
                std::size_t prefix_len = (trimmed.rfind("fun ", 0) == 0) ? 4 : 7;
                std::string rest = trimmed.substr(prefix_len);
                std::size_t paren_pos = rest.find('(');

                DocItem item;
                if (paren_pos != std::string::npos) {
                    item.func_name = trim(rest.substr(0, paren_pos));
                } else {
                    item.func_name = trim(rest);
                }
                item.signature = trimmed;
                item.doc_comment = accumulated_doc;

                if (!item.func_name.empty()) {
                    page.items.push_back(item);
                }

                accumulated_doc.clear();
            } else if (!trimmed.empty() && trimmed.front() != '#') {
                // Non-comment, non-function line clears doc accumulator if not adjacent
                accumulated_doc.clear();
            }
        }

        return page;
    }

    static std::string generate_html(const DocPage& page) {
        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html>\n<head>\n"
             << "<meta charset=\"UTF-8\">\n"
             << "<title>Garuda Doc - " << escape_html(page.file_name) << "</title>\n"
             << "<style>\n"
             << "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; margin: 40px; background: #0f172a; color: #f8fafc; }\n"
             << "h1 { color: #38bdf8; border-bottom: 2px solid #334155; padding-bottom: 10px; }\n"
             << ".doc-card { background: #1e293b; border: 1px solid #334155; border-radius: 8px; padding: 20px; margin-bottom: 20px; }\n"
             << ".func-name { font-size: 1.25rem; font-weight: bold; color: #4ade80; font-family: monospace; }\n"
             << ".signature { background: #0f172a; padding: 8px 12px; border-radius: 4px; color: #f1f5f9; font-family: monospace; margin: 10px 0; }\n"
             << ".doc-comment { color: #94a3b8; white-space: pre-wrap; line-height: 1.5; }\n"
             << "</style>\n</head>\n<body>\n";

        html << "<h1>📄 " << escape_html(page.file_name) << "</h1>\n";

        if (page.items.empty()) {
            html << "<p><em>Tidak ada dokumentasi fungsi dalam berkas ini.</em></p>\n";
        } else {
            for (const auto& item : page.items) {
                html << "<div class=\"doc-card\">\n"
                     << "  <div class=\"func-name\">fn " << escape_html(item.func_name) << "</div>\n"
                     << "  <div class=\"signature\">" << escape_html(item.signature) << "</div>\n"
                     << "  <div class=\"doc-comment\">" << escape_html(item.doc_comment) << "</div>\n"
                     << "</div>\n";
            }
        }

        html << "</body>\n</html>\n";
        return html.str();
    }

    static std::string generate_all(const std::string& dir_path) {
        std::vector<DocPage> pages;
        if (fs::exists(dir_path) && fs::is_directory(dir_path)) {
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                if (entry.is_regular_file() && (entry.path().extension() == ".grd" || entry.path().extension() == ".garuda")) {
                    pages.push_back(parse_file(entry.path().string()));
                }
            }
        }

        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html>\n<head>\n"
             << "<meta charset=\"UTF-8\">\n"
             << "<title>Garuda API Documentation</title>\n"
             << "<style>\n"
             << "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; margin: 40px; background: #0f172a; color: #f8fafc; }\n"
             << "h1 { color: #38bdf8; border-bottom: 2px solid #334155; padding-bottom: 10px; }\n"
             << "h2 { color: #fbbf24; margin-top: 30px; }\n"
             << ".doc-card { background: #1e293b; border: 1px solid #334155; border-radius: 8px; padding: 20px; margin-bottom: 20px; }\n"
             << ".func-name { font-size: 1.25rem; font-weight: bold; color: #4ade80; font-family: monospace; }\n"
             << ".signature { background: #0f172a; padding: 8px 12px; border-radius: 4px; color: #f1f5f9; font-family: monospace; margin: 10px 0; }\n"
             << ".doc-comment { color: #94a3b8; white-space: pre-wrap; line-height: 1.5; }\n"
             << "</style>\n</head>\n<body>\n"
             << "<h1>🦅 Garuda API Documentation Index</h1>\n";

        for (const auto& page : pages) {
            html << "<h2>📄 " << escape_html(page.file_name) << "</h2>\n";
            for (const auto& item : page.items) {
                html << "<div class=\"doc-card\">\n"
                     << "  <div class=\"func-name\">fn " << escape_html(item.func_name) << "</div>\n"
                     << "  <div class=\"signature\">" << escape_html(item.signature) << "</div>\n"
                     << "  <div class=\"doc-comment\">" << escape_html(item.doc_comment) << "</div>\n"
                     << "</div>\n";
            }
        }

        html << "</body>\n</html>\n";
        return html.str();
    }

private:
    static std::string trim(const std::string& str) {
        std::size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        std::size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    static std::string escape_html(const std::string& str) {
        std::string res;
        res.reserve(str.length() * 2);
        for (char c : str) {
            switch (c) {
                case '<':  res += "&lt;"; break;
                case '>':  res += "&gt;"; break;
                case '&':  res += "&amp;"; break;
                case '"':  res += "&quot;"; break;
                default:   res += c; break;
            }
        }
        return res;
    }
};

} // namespace garuda::beam
