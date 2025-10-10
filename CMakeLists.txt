cmake_minimum_required(VERSION 3.14)
project(httpkit)

# ----- options ----- #

option(HTTPKIT_BUILD_TESTS "build tests" ON)
option(HTTPKIT_INSTALL "install headers and libs" ON)
option(HTTPKIT_HOLD_DEPS "do not update existing deps" OFF)

# ----- dependencies ----- #

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ${HTTPKIT_HOLD_DEPS})

if(NOT HTTPKIT_DEPS_DIR)
    get_filename_component(HTTPKIT_DEPS_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps ABSOLUTE)
    message(STATUS "`HTTPKIT_DEPS_DIR` is not set. set `HTTPKIT_DEPS_DIR` to `${HTTPKIT_DEPS_DIR}` by default.")
endif()

macro(httpkit_import_git_dep name repo commit)
    FetchContent_Declare(${name}
        GIT_REPOSITORY ${repo}
        GIT_TAG ${commit}
        GIT_SHALLOW TRUE
        SOURCE_DIR ${HTTPKIT_DEPS_DIR}/${name}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${name}-build
        SUBBUILD_DIR ${HTTPKIT_DEPS_DIR}/${name}-subbuild)
    FetchContent_MakeAvailable(${name})
endmacro()

if(NOT TARGET cutils_static)
    set(CUTILS_DEPS_DIR ${HTTPKIT_DEPS_DIR})
    set(CUTILS_BUILD_TESTS OFF)
    set(CUTILS_INSTALL ${HTTPKIT_INSTALL})
    set(CUTILS_HOLD_DEPS ${HTTPKIT_HOLD_DEPS})
    if(HTTPKIT_DEP_CUTILS_SOURCE_DIR)
        add_subdirectory(${HTTPKIT_DEP_CUTILS_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/cutils-build)
        set(cutils_SOURCE_DIR ${HTTPKIT_DEP_CUTILS_SOURCE_DIR})
    else()
        httpkit_import_git_dep(cutils "https://github.com/ouonline/cutils.git" master)
    endif()
endif()

# ----- targets ----- #

file(GLOB __HTTPKIT_SRC__ ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(httpkit_static STATIC ${__HTTPKIT_SRC__})
unset(__HTTPKIT_SRC__)

target_include_directories(httpkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(httpkit_static PUBLIC cutils_static)

if(MSVC)
    target_compile_options(httpkit_static PRIVATE /W4)
else()
    target_compile_options(httpkit_static PRIVATE -Wall -Wextra -Werror -Wno-implicit-fallthrough)
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
