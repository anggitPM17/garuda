#define GARUDA_GRDMIX_NO_MAIN
#include "../src/grdmix.cpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace garuda::cli;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLI — SPRINT 4.4 PACKAGE MANAGER GRDMIX TEST SUITE (8 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_base = fs::temp_directory_path() / "grdmix_test_workspace";
    fs::remove_all(tmp_base);
    fs::create_directories(tmp_base);

    std::string proj_name = "test_project";
    fs::path proj_dir = tmp_base / proj_name;
    fs::remove_all(proj_dir);

    // === Test 1: grdmix new test_project ===
    std::cout << "[TEST 1/8] Running grdmix new test_project test...\n";
    int res1 = Grdmix::new_project(proj_name, tmp_base);
    assert(res1 == 0);
    assert(fs::exists(proj_dir));
    std::cout << "  ✓ Test 1: grdmix new test_project PASS\n";

    // === Test 2: src/utama.grd exists ===
    std::cout << "[TEST 2/8] Running src/utama.grd verification test...\n";
    fs::path main_grd = proj_dir / "src" / "utama.grd";
    assert(fs::exists(main_grd));

    std::ifstream main_ifs(main_grd);
    std::string main_content((std::istreambuf_iterator<char>(main_ifs)), std::istreambuf_iterator<char>());
    main_ifs.close();

    assert(main_content.find("modul Utama") != std::string::npos);
    assert(main_content.find("fungsi utama()") != std::string::npos);
    std::cout << "  ✓ Test 2: src/utama.grd verification PASS\n";

    // === Test 3: grdmix.toml metadata exists ===
    std::cout << "[TEST 3/8] Running grdmix.toml verification test...\n";
    fs::path toml_path = proj_dir / "grdmix.toml";
    assert(fs::exists(toml_path));

    std::ifstream toml_ifs(toml_path);
    std::string toml_content((std::istreambuf_iterator<char>(toml_ifs)), std::istreambuf_iterator<char>());
    toml_ifs.close();

    assert(toml_content.find("name = \"test_project\"") != std::string::npos);
    std::cout << "  ✓ Test 3: grdmix.toml verification PASS\n";

    // === Test 4: test/test_utama.grd exists ===
    std::cout << "[TEST 4/8] Running test/test_utama.grd verification test...\n";
    fs::path test_grd = proj_dir / "test" / "test_utama.grd";
    assert(fs::exists(test_grd));
    std::cout << "  ✓ Test 4: test/test_utama.grd verification PASS\n";

    // === Test 5: grdmix compile ===
    std::cout << "[TEST 5/8] Running grdmix compile test...\n";
    int res5 = Grdmix::compile(proj_dir);
    assert(res5 == 0);
    assert(fs::exists(proj_dir / "src" / "utama.gvm")); // Bytecode .gvm compiled!
    std::cout << "  ✓ Test 5: grdmix compile PASS\n";

    // === Test 6: grdmix run ===
    std::cout << "[TEST 6/8] Running grdmix run test...\n";
    int res6 = Grdmix::run(proj_dir);
    assert(res6 == 0);
    std::cout << "  ✓ Test 6: grdmix run PASS\n";

    // === Test 7: grdmix test ===
    std::cout << "[TEST 7/8] Running grdmix test test...\n";
    int res7 = Grdmix::test(proj_dir);
    assert(res7 == 0); // All tests in test/ PASS!
    std::cout << "  ✓ Test 7: grdmix test PASS\n";

    // === Test 8: Duplicate Name Error Handling ===
    std::cout << "[TEST 8/8] Running Duplicate Name Error Handling test...\n";
    int res8 = Grdmix::new_project(proj_name, tmp_base);
    assert(res8 == 1); // Fails gracefully when folder already exists!
    std::cout << "  ✓ Test 8: Duplicate Name Error Handling PASS\n";

    // Cleanup
    fs::remove_all(tmp_base);

    std::cout << "\n🎉 ALL 8 PACKAGE MANAGER GRDMIX TESTS PASSED PERFECTLY!\n";
    return 0;
}
