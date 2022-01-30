cmake_minimum_required(VERSION 3.10)
project(httpkit-test)

add_executable(test_httpkit test.c)
target_link_libraries(test_httpkit PRIVATE httpkit_static)
