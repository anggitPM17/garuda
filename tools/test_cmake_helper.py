import os
import tempfile
import cmake_helper

def test_cmake_helper():
    sample_cmake = """
  add_executable(garuda_tests tests/test_core.cpp)
foreach(t garuda_tests garuda_bench_storage)
  target_link_libraries(${t} PRIVATE garuda)
endforeach()
enable_testing()
add_test(NAME garuda_core COMMAND garuda_tests)
endif()
"""
    with tempfile.NamedTemporaryFile(mode='w+', delete=False, suffix='.txt') as tmp:
        tmp.write(sample_cmake)
        tmp_path = tmp.name

    try:
        ok = cmake_helper.add_test_suite(tmp_path, "garuda_sample_tests", "test_sample.cpp")
        assert ok, "add_test_suite must return True"

        with open(tmp_path, 'r') as f:
            res = f.read()

        assert "add_executable(garuda_sample_tests tests/test_sample.cpp)" in res
        assert "garuda_sample_tests garuda_bench_storage" in res
        assert "add_test(NAME garuda_sample COMMAND garuda_sample_tests)" in res
        print("🎉 cmake_helper unit test PASSED!")
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

if __name__ == '__main__':
    test_cmake_helper()
