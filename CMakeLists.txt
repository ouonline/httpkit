cmake_minimum_required(VERSION 3.10)
project(httpkit)

# ----- options ----- #

option(HTTPKIT_BUILD_TESTS "build tests" ON)
option(HTTPKIT_INSTALL "install headers and libs" ON)

# ----- targets ----- #

file(GLOB HTTPKIT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(httpkit_static STATIC ${HTTPKIT_SRC})
target_include_directories(httpkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ----- dependencies ----- #

if(NOT cutils_static)
    set(CUTILS_BUILD_TESTS OFF CACHE BOOL "")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cutils ${CMAKE_CURRENT_BINARY_DIR}/cutils)
    target_link_libraries(httpkit_static PUBLIC cutils_static)
endif()

# ----- installations ----- #

if(HTTPKIT_INSTALL)
    install(DIRECTORY include DESTINATION .)
    install(TARGETS httpkit_static DESTINATION lib)
endif()

# ----- tests ----- #

if(HTTPKIT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
