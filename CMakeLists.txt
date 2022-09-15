cmake_minimum_required(VERSION 3.14)
project(httpkit)

# ----- options ----- #

option(HTTPKIT_BUILD_TESTS "build tests" ON)
option(HTTPKIT_INSTALL "install headers and libs" ON)

# ----- targets ----- #

file(GLOB HTTPKIT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(httpkit_static STATIC ${HTTPKIT_SRC})
target_include_directories(httpkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(httpkit_static PUBLIC cxx_std_11)

# ----- dependencies ----- #

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

if(NOT HTTPKIT_DEPS_DIR)
    get_filename_component(HTTPKIT_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps ABSOLUTE)
    message("-- `HTTPKIT_DEPS_DIR` is not set. set `HTTPKIT_DEPS_DIR` to `${HTTPKIT_DEPS_DIR}`.")
endif()

if(NOT TARGET cutils_static)
    set(CUTILS_BUILD_TESTS OFF CACHE BOOL "")
    set(CUTILS_DEPS_DIR ${HTTPKIT_DEPS_DIR})
    FetchContent_Declare(cutils
        GIT_REPOSITORY "https://github.com/ouonline/cutils.git"
        GIT_TAG master
        SOURCE_DIR ${HTTPKIT_DEPS_DIR}/cutils
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/cutils-build
        SUBBUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/cutils-subbuild)
    FetchContent_MakeAvailable(cutils)
endif()

target_link_libraries(httpkit_static PUBLIC cutils_static)

# ----- installations ----- #

if(HTTPKIT_INSTALL)
    install(DIRECTORY include DESTINATION .)
    install(TARGETS httpkit_static DESTINATION lib)
endif()

# ----- tests ----- #

if(HTTPKIT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
