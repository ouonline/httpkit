cmake_minimum_required(VERSION 3.10)
project(httpkit-test)

file(GLOB_RECURSE __HTTPKIT_TESTS_SRC__ *.c)
add_executable(test_httpkit ${__HTTPKIT_TESTS_SRC__})
unset(__HTTPKIT_TESTS_SRC__)

target_link_libraries(test_httpkit PRIVATE httpkit_static)

if(MSVC)
    target_compile_options(test_httpkit PRIVATE /W4)
else()
    target_compile_options(test_httpkit PRIVATE -Wall -Wextra -Werror)
endif()
