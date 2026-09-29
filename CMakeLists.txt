cmake_minimum_required(VERSION 3.22)

project(dibeast VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif ()

option(DIBEAST_SANITIZERS "Build with AddressSanitizer + UBSan" OFF)
if (DIBEAST_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif ()

# libdibeast is shared, so everything linked into it (Boost's static libs too) must be PIC.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/Boost.cmake)

# After Boost: our warnings apply to our code only.
add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(examples)

find_program(CLANG_FORMAT NAMES clang-format-15 clang-format)
if (CLANG_FORMAT)
    file(GLOB_RECURSE DIBEAST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/include/*.h
        ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/examples/*.h ${CMAKE_SOURCE_DIR}/examples/*.cpp
        ${CMAKE_SOURCE_DIR}/apps/*.h ${CMAKE_SOURCE_DIR}/apps/*.cpp)
    add_custom_target(format COMMAND ${CLANG_FORMAT} -i ${DIBEAST_SOURCES})
    add_custom_target(format-check COMMAND ${CLANG_FORMAT} --dry-run --Werror ${DIBEAST_SOURCES})
endif ()
