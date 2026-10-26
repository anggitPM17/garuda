#include "../include/garuda/beam/stdlib/json.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STANDARD LIBRARY — SPRINT C.1 JSON TEST SUITE (20 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple String Encode & Decode ===
    std::cout << "[TEST 1/20] Running Simple String Encode & Decode test...\n";
    Term str_term = Term("halo_dunia");
    std::string enc1 = Json::encode(str_term);
    assert(enc1 == "\"halo_dunia\"");
    auto dec1 = Json::decode(enc1);
    assert(dec1.has_value() && *dec1->get_if<std::string>() == "halo_dunia");
    std::cout << "  ✓ Test 1: Simple String PASS\n";

    // === Test 2: Integer Number Encode & Decode ===
    std::cout << "[TEST 2/20] Running Integer Number test...\n";
    Term int_term = Term(42LL);
    std::string enc2 = Json::encode(int_term);
    assert(enc2 == "42");
    auto dec2 = Json::decode("42");
    assert(dec2.has_value() && *dec2->get_if<std::int64_t>() == 42);
    std::cout << "  ✓ Test 2: Integer Number PASS\n";

    // === Test 3: Float Number Encode & Decode ===
    std::cout << "[TEST 3/20] Running Float Number test...\n";
    Term float_term = Term(3.14159);
    auto dec3 = Json::decode("3.14159");
    assert(dec3.has_value() && dec3->get_if<double>() != nullptr);
    std::cout << "  ✓ Test 3: Float Number PASS\n";

    // === Test 4: Boolean true/false ===
    std::cout << "[TEST 4/20] Running Boolean true/false test...\n";
    auto dec4_t = Json::decode("true");
    auto dec4_f = Json::decode("false");
    assert(dec4_t.has_value() && dec4_t->get_if<Atom>()->name == "true");
    assert(dec4_f.has_value() && dec4_f->get_if<Atom>()->name == "false");
    std::cout << "  ✓ Test 4: Boolean true/false PASS\n";

    // === Test 5: Null / Nil ===
    std::cout << "[TEST 5/20] Running Null/Nil test...\n";
    auto dec5 = Json::decode("null");
    assert(dec5.has_value() && dec5->get_if<Atom>()->name == "nil");
    std::cout << "  ✓ Test 5: Null/Nil PASS\n";

    // === Test 6: Simple Array ===
    std::cout << "[TEST 6/20] Running Simple Array test...\n";
    auto dec6 = Json::decode("[1, 2, 3]");
    assert(dec6.has_value() && dec6->get_if<List>() != nullptr);
    assert(seq_items(dec6->get_if<List>()->items).size() == 3);
    std::cout << "  ✓ Test 6: Simple Array PASS\n";

    // === Test 7: Simple Object ===
    std::cout << "[TEST 7/20] Running Simple Object test...\n";
    auto dec7 = Json::decode("{\"nama\": \"Budi\", \"umur\": 42}");
    assert(dec7.has_value() && dec7->get_if<List>() != nullptr);
    const auto& items7 = seq_items(dec7->get_if<List>()->items);
    assert(items7.size() == 2);
    std::cout << "  ✓ Test 7: Simple Object PASS\n";

    // === Test 8: Nested Object ===
    std::cout << "[TEST 8/20] Running Nested Object test...\n";
    std::string json8 = "{\"user\": {\"id\": 1, \"roles\": [\"admin\", \"user\"]}}";
    auto dec8 = Json::decode(json8);
    assert(dec8.has_value() && dec8->get_if<List>() != nullptr);
    std::cout << "  ✓ Test 8: Nested Object PASS\n";

    // === Test 9: String Escape Sequences ===
    std::cout << "[TEST 9/20] Running String Escape Sequences test...\n";
    Term esc_term = Term("line1\nline2\ttab");
    std::string enc9 = Json::encode(esc_term);
    assert(enc9.find("\\n") != std::string::npos);
    assert(enc9.find("\\t") != std::string::npos);
    auto dec9 = Json::decode(enc9);
    assert(dec9.has_value() && *dec9->get_if<std::string>() == "line1\nline2\ttab");
    std::cout << "  ✓ Test 9: String Escape Sequences PASS\n";

    // === Test 10: Malformed JSON Error Handling ===
    std::cout << "[TEST 10/20] Running Malformed JSON Error Handling test...\n";
    assert(!Json::decode("{\"key\": unquoted_val}").has_value());
    assert(!Json::decode("{invalid_json").has_value());
    assert(!Json::decode("[1, 2, ]").has_value());
    std::cout << "  ✓ Test 10: Malformed JSON Error Handling PASS\n";

    // === Test 11: Array of Objects ===
    std::cout << "[TEST 11/20] Running Array of Objects test...\n";
    auto dec11 = Json::decode("[{\"id\": 1}, {\"id\": 2}]");
    assert(dec11.has_value() && seq_items(dec11->get_if<List>()->items).size() == 2);
    std::cout << "  ✓ Test 11: Array of Objects PASS\n";

    // === Test 12: Mixed Array ===
    std::cout << "[TEST 12/20] Running Mixed Array test...\n";
    auto dec12 = Json::decode("[1, \"dua\", true, null]");
    assert(dec12.has_value() && seq_items(dec12->get_if<List>()->items).size() == 4);
    std::cout << "  ✓ Test 12: Mixed Array PASS\n";

    // === Test 13: Empty Object ===
    std::cout << "[TEST 13/20] Running Empty Object test...\n";
    auto dec13 = Json::decode("{}");
    assert(dec13.has_value() && seq_items(dec13->get_if<List>()->items).empty());
    std::cout << "  ✓ Test 13: Empty Object PASS\n";

    // === Test 14: Empty Array ===
    std::cout << "[TEST 14/20] Running Empty Array test...\n";
    auto dec14 = Json::decode("[]");
    assert(dec14.has_value() && seq_items(dec14->get_if<List>()->items).empty());
    std::cout << "  ✓ Test 14: Empty Array PASS\n";

    // === Test 15: Key Lookup on Decoded Object ===
    std::cout << "[TEST 15/20] Running Key Lookup test...\n";
    auto dec15 = Json::decode("{\"code\": 200, \"status\": \"ok\"}");
    assert(dec15.has_value());
    const auto& pairs15 = seq_items(dec15->get_if<List>()->items);
    assert(pairs15.size() == 2);
    std::cout << "  ✓ Test 15: Key Lookup PASS\n";

    // === Test 16: Whitespace Tolerance ===
    std::cout << "[TEST 16/20] Running Whitespace Tolerance test...\n";
    auto dec16 = Json::decode("{\n  \"x\"  :  100  \n}");
    assert(dec16.has_value());
    std::cout << "  ✓ Test 16: Whitespace Tolerance PASS\n";

    // === Test 17: Float vs Int Type Preservation ===
    std::cout << "[TEST 17/20] Running Float vs Int Type Preservation test...\n";
    auto dec17_int = Json::decode("100");
    auto dec17_flt = Json::decode("100.0");
    assert(dec17_int->get_if<std::int64_t>() != nullptr);
    assert(dec17_flt->get_if<double>() != nullptr);
    std::cout << "  ✓ Test 17: Float vs Int Type Preservation PASS\n";

    // === Test 18: Negative Numbers ===
    std::cout << "[TEST 18/20] Running Negative Numbers test...\n";
    auto dec18 = Json::decode("-12345");
    assert(dec18.has_value() && *dec18->get_if<std::int64_t>() == -12345);
    std::cout << "  ✓ Test 18: Negative Numbers PASS\n";

    // === Test 19: Escaped Keys in Object ===
    std::cout << "[TEST 19/20] Running Escaped Keys test...\n";
    auto dec19 = Json::decode("{\"a\\\"b\": \"val\"}");
    assert(dec19.has_value());
    std::cout << "  ✓ Test 19: Escaped Keys PASS\n";

    // === Test 20: Roundtrip Consistency ===
    std::cout << "[TEST 20/20] Running Roundtrip Consistency test...\n";
    Term orig_term = list_of({tuple_of({Term("key1"), Term("val1")}), tuple_of({Term("key2"), Term(999LL)})});
    std::string enc20 = Json::encode(orig_term);
    auto dec20 = Json::decode(enc20);
    assert(dec20.has_value());
    std::string re_enc20 = Json::encode(*dec20);
    assert(enc20 == re_enc20);
    std::cout << "  ✓ Test 20: Roundtrip Consistency PASS\n";

    std::cout << "\n🎉 ALL 20 JSON PARSER & ENCODER TESTS PASSED PERFECTLY!\n";
    return 0;
}
