cmake_minimum_required(VERSION 3.6)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(VirtioFS_test)
find_package(IncludeOS REQUIRED)
include(os)

os_add_executable(test "test" test.cpp)
os_add_stdout(test default_stdout)

configure_file(test.sh ${CMAKE_CURRENT_BINARY_DIR})