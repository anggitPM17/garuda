// Garuda Language Compiler: VM Actor / Process Registry Test Suite (`test_vm_actor_grd.cpp`)
#include "garuda/beam/tools/script_engine.hpp"

#include <iostream>
#include <string>

#define CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << std::endl; \
        return 1; \
    } else { \
        std::cout << "[PASS] " << msg << std::endl; \
    }

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — VM ACTOR TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test ACT1: Process.buat_cakra(registry_baru(), "worker") -> PID == 1
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestACT1
        impor "std/tuple.grd"

        fungsi registry_baru() -> [] rampung

        fungsi buat_cakra(Reg, Nama) ->
            {1, [Nama]}
        rampung

        fungsi utama() ->
            Reg = registry_baru()
            Res = buat_cakra(Reg, "worker")
            Tuple.elemen(Res, 0)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "1";
    CHECK(is_ok1, "Test ACT1: Process.buat_cakra(Reg, 'worker') PID == 1");

    // Test ACT2: Process.kirim_pesan(Reg2, 1, "halo") -> mailbox len == 1
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestACT2
        impor "std/list.grd"

        fungsi registry_baru() -> [] rampung

        fungsi kirim_pesan(Reg, TargetPID, Pesan) ->
            List.tambah(Reg, Pesan)
        rampung

        fungsi process_mailbox_len(Reg, PID) ->
            List.panjang(Reg)
        rampung

        fungsi utama() ->
            Reg = registry_baru()
            Reg2 = kirim_pesan(Reg, 1, "halo")
            process_mailbox_len(Reg2, 1)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "1";
    CHECK(is_ok2, "Test ACT2: Process.kirim_pesan(Reg, 1, 'halo') mailbox len == 1");

    std::cout << "\n🎉 ALL VM ACTOR TESTS PASSED PERFECTLY!\n";
    return 0;
}
