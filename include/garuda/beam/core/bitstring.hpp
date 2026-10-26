#pragma once
// Garuda BEAM-Core: Binary Bitstring Constructor & Pattern Matcher (`bitstring.hpp`)
// Fitur DEWA Erlang: Pattern Matching Biner Zero-Copy (<<Size:16, Cmd:8, Payload/binary>>) untuk protokol jaringan.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

namespace garuda::beam {

class Bitstring {
public:
    Bitstring() = default;
    explicit Bitstring(std::string data) : buffer_(std::move(data)) {}

    void append_uint16(std::uint16_t val) {
        std::uint16_t net_val = htons(val);
        buffer_.append(reinterpret_cast<const char*>(&net_val), 2);
    }

    void append_uint32(std::uint32_t val) {
        std::uint32_t net_val = htonl(val);
        buffer_.append(reinterpret_cast<const char*>(&net_val), 4);
    }

    void append_binary(std::string_view data) {
        buffer_.append(data);
    }

    const std::string& buffer() const { return buffer_; }
    std::size_t byte_size() const { return buffer_.size(); }

    // Pattern Matcher Zero-Copy
    static bool match_header_payload(std::string_view bin, std::uint16_t& out_size, std::uint8_t& out_cmd, std::string_view& out_payload) {
        if (bin.size() < 3) return false;
        std::uint16_t raw_size = 0;
        std::memcpy(&raw_size, bin.data(), 2);
        out_size = ntohs(raw_size);
        out_cmd = static_cast<std::uint8_t>(bin[2]);
        out_payload = bin.substr(3);
        return true;
    }

private:
    std::string buffer_;
};

} // namespace garuda::beam
