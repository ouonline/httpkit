cmake_minimum_required(VERSION 3.10)
project(httpkit-test)

file(GLOB __TEST_SRC__ *.c)
add_executable(test_httpkit ${__TEST_SRC__})
target_link_libraries(test_httpkit PRIVATE httpkit_static)
