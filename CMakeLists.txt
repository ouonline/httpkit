cmake_minimum_required(VERSION 3.14)
project(httpkit)

option(HTTPKIT_BUILD_TESTS "build tests" ON)

file(GLOB HTTPKIT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(httpkit_static STATIC ${HTTPKIT_SRC})
target_include_directories(httpkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CUTILS_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cutils ${CMAKE_CURRENT_BINARY_DIR}/cutils)
target_link_libraries(httpkit_static PUBLIC cutils_static)

if(HTTPKIT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
