#include <iostream>
#include <cassert>
#include <string>

#include "garuda/beam/sys/sandbox.hpp"

using namespace garuda::beam::sys;

void test_permissive_policy() {
    GvmSandbox::reset_policy();

    assert(GvmSandbox::check_fs_read("etc/config.json"));
    assert(GvmSandbox::check_fs_write("var/log/app.log"));
    assert(GvmSandbox::check_network("api.richat.my.id", 443));
    assert(GvmSandbox::check_exec("ls -l"));

    std::cout << "[PASS] test_permissive_policy" << std::endl;
}

void test_strictly_isolated_policy() {
    GvmSandbox::set_thread_policy(SandboxPolicy::StrictlyIsolated());

    // FS Read
    bool caught_read = false;
    try {
        GvmSandbox::check_fs_read("/etc/passwd");
    } catch (const SecurityException&) {
        caught_read = true;
    }
    assert(caught_read && "StrictlyIsolated should block FS read");

    // FS Write
    bool caught_write = false;
    try {
        GvmSandbox::check_fs_write("/tmp/malicious.sh");
    } catch (const SecurityException&) {
        caught_write = true;
    }
    assert(caught_write && "StrictlyIsolated should block FS write");

    // Network
    bool caught_net = false;
    try {
        GvmSandbox::check_network("untrusted-server.com", 8080);
    } catch (const SecurityException&) {
        caught_net = true;
    }
    assert(caught_net && "StrictlyIsolated should block network");

    // Exec
    bool caught_exec = false;
    try {
        GvmSandbox::check_exec("rm -rf /");
    } catch (const SecurityException&) {
        caught_exec = true;
    }
    assert(caught_exec && "StrictlyIsolated should block exec");

    GvmSandbox::reset_policy();
    std::cout << "[PASS] test_strictly_isolated_policy" << std::endl;
}

void test_allowed_paths_whitelist() {
    SandboxPolicy custom;
    custom.allow_fs_read = false;
    custom.allowed_paths = {"/allowed/dir/", "C:/allowed/data/"};

    GvmSandbox::set_thread_policy(custom);

    // Whitelisted path should pass
    assert(GvmSandbox::check_fs_read("/allowed/dir/data.txt"));
    assert(GvmSandbox::check_fs_read("C:/allowed/data/user.json"));

    // Non-whitelisted path should fail
    bool caught = false;
    try {
        GvmSandbox::check_fs_read("/forbidden/system.dll");
    } catch (const SecurityException&) {
        caught = true;
    }
    assert(caught && "Non-whitelisted path read should be blocked");

    GvmSandbox::reset_policy();
    std::cout << "[PASS] test_allowed_paths_whitelist" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GVM SANDBOX TESTS ---" << std::endl;
    test_permissive_policy();
    test_strictly_isolated_policy();
    test_allowed_paths_whitelist();
    std::cout << "--- ALL GVM SANDBOX TESTS PASSED 100% ---" << std::endl;
    return 0;
}
