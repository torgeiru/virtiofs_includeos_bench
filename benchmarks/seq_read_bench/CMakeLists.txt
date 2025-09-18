cmake_minimum_required(VERSION 3.6)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(Sequential_read_bench)
find_package(IncludeOS REQUIRED)
include(os)

os_add_executable(seq_read_bench "seq_read_bench" seq_read_bench.cpp)
os_add_stdout(seq_read_bench default_stdout)
os_add_drivers(seq_read_bench virtiofs)

configure_file(seq_read_bench.py ${CMAKE_CURRENT_BINARY_DIR})
configure_file(vm.json ${CMAKE_CURRENT_BINARY_DIR})