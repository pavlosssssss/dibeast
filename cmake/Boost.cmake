# Boost is fetched from the official "-cmake" release archive, which ships Boost's own CMake build.
# Only the libraries we use (and their dependencies) are configured.
include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES asio beast process url)
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_PROCESS_USE_STD_FS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-cmake.tar.xz
    URL_HASH SHA256=9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
)
FetchContent_MakeAvailable(Boost)
