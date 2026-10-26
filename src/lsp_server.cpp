// Garuda Language Compiler: LSP Server Executable (`lsp_server.cpp`)
// Mengimplementasikan Server LSP Komunikasi Stdio JSON-RPC 2.0 untuk VS Code.
#include "../include/garuda/compiler/lsp_engine.hpp"

#include <iostream>
#include <string>

using namespace garuda::compiler;

int main() {
    LspEngine lsp;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.rfind("Content-Length:", 0) == 0) {
            std::size_t content_length = std::stoull(line.substr(15));
            std::getline(std::cin, line); // Empty line \r\n

            std::string body;
            body.resize(content_length);
            std::cin.read(&body[0], content_length);

            std::string response = lsp.handle_request(body);
            if (!response.empty()) {
                std::cout << "Content-Length: " << response.length() << "\r\n\r\n" << response << std::flush;
            }
        }
    }
    return 0;
}
