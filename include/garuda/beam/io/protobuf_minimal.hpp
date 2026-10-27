#pragma once
// Garuda Production Infrastructure: Minimal Native Protobuf Wire Format Engine (`protobuf_minimal.hpp`)
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace garuda::beam {

enum class ProtoWireType : std::uint8_t {
    Varint = 0,
    LengthDelimited = 2
};

class ProtoWriter {
public:
    ProtoWriter() = default;

    void write_varint_raw(std::uint64_t value) {
        while (value >= 0x80) {
            buffer_.push_back(static_cast<std::uint8_t>((value & 0x7F) | 0x80));
            value >>= 7;
        }
        buffer_.push_back(static_cast<std::uint8_t>(value & 0x7F));
    }

    void write_tag(int field_number, ProtoWireType wire_type) {
        std::uint64_t tag = (static_cast<std::uint64_t>(field_number) << 3) | static_cast<std::uint8_t>(wire_type);
        write_varint_raw(tag);
    }

    void write_varint(int field_number, std::uint64_t value) {
        write_tag(field_number, ProtoWireType::Varint);
        write_varint_raw(value);
    }

    void write_string(int field_number, const std::string& value) {
        write_tag(field_number, ProtoWireType::LengthDelimited);
        write_varint_raw(value.length());
        for (char c : value) {
            buffer_.push_back(static_cast<std::uint8_t>(c));
        }
    }

    void write_bool(int field_number, bool value) {
        write_varint(field_number, value ? 1 : 0);
    }

    const std::vector<std::uint8_t>& get_buffer() const {
        return buffer_;
    }

    void clear() {
        buffer_.clear();
    }

private:
    std::vector<std::uint8_t> buffer_;
};

class ProtoReader {
public:
    ProtoReader(const std::vector<std::uint8_t>& buf)
        : data_(buf.data()), size_(buf.size()), pos_(0) {}

    ProtoReader(const std::uint8_t* data, std::size_t size)
        : data_(data), size_(size), pos_(0) {}

    bool has_more() const {
        return pos_ < size_;
    }

    std::uint64_t read_varint_raw() {
        std::uint64_t result = 0;
        int shift = 0;
        while (pos_ < size_) {
            std::uint8_t byte = data_[pos_++];
            result |= static_cast<std::uint64_t>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                return result;
            }
            shift += 7;
            if (shift >= 64) {
                throw std::runtime_error("Varint overflow in ProtoReader");
            }
        }
        throw std::runtime_error("Unexpected EOF reading varint in ProtoReader");
    }

    bool read_tag(int& field_number, ProtoWireType& wire_type) {
        if (!has_more()) return false;
        std::uint64_t tag = read_varint_raw();
        field_number = static_cast<int>(tag >> 3);
        wire_type = static_cast<ProtoWireType>(tag & 0x07);
        return true;
    }

    std::uint64_t read_varint(int expected_field_number) {
        int field_num = 0;
        ProtoWireType wire_type = ProtoWireType::Varint;
        if (!read_tag(field_num, wire_type) || field_num != expected_field_number || wire_type != ProtoWireType::Varint) {
            throw std::runtime_error("Mismatched field tag for read_varint");
        }
        return read_varint_raw();
    }

    std::string read_string(int expected_field_number) {
        int field_num = 0;
        ProtoWireType wire_type = ProtoWireType::LengthDelimited;
        if (!read_tag(field_num, wire_type) || field_num != expected_field_number || wire_type != ProtoWireType::LengthDelimited) {
            throw std::runtime_error("Mismatched field tag for read_string");
        }

        std::uint64_t len = read_varint_raw();
        if (pos_ + len > size_) {
            throw std::runtime_error("Unexpected EOF reading string payload in ProtoReader");
        }

        std::string res(reinterpret_cast<const char*>(data_ + pos_), static_cast<std::size_t>(len));
        pos_ += static_cast<std::size_t>(len);
        return res;
    }

    bool read_bool(int expected_field_number) {
        std::uint64_t v = read_varint(expected_field_number);
        return v != 0;
    }

private:
    const std::uint8_t* data_;
    std::size_t size_;
    std::size_t pos_;
};

} // namespace garuda::beam
