import sys

def add_test_suite(cmake_path, exec_name, cpp_file):
    with open(cmake_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Check if target already in foreach list
    if f" {exec_name} " in content or f"\n{exec_name} " in content:
        print(f"Target '{exec_name}' already registered in foreach list.")
        return True

    # Insert add_executable before foreach(t
    exec_line = f"  add_executable({exec_name} tests/{cpp_file})\n"
    foreach_idx = content.find("foreach(t ")
    if foreach_idx != -1:
        content = content[:foreach_idx] + exec_line + content[foreach_idx:]

    # Insert executable into foreach list (right before garuda_bench_storage)
    bench_mark = "garuda_bench_storage"
    bench_idx = content.find(bench_mark)
    if bench_idx != -1:
        content = content[:bench_idx] + f"{exec_name} " + content[bench_idx:]

    # Add add_test entry before endif()
    short_name = exec_name.replace("_tests", "").replace("garuda_", "")
    test_entry = f"  add_test(NAME garuda_{short_name} COMMAND {exec_name})\n"
    endif_idx = content.rfind("endif()")
    if endif_idx != -1 and test_entry not in content:
        content = content[:endif_idx] + test_entry + content[endif_idx:]

    with open(cmake_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Successfully added '{exec_name}' ({cpp_file}) to CMakeLists.txt")
    return True

if __name__ == '__main__':
    if len(sys.argv) >= 4:
        add_test_suite(sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        print("Usage: python cmake_helper.py <path_to_CMakeLists.txt> <exec_name> <cpp_file>")
