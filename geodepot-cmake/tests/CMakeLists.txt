cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-cmake-test VERSION 0.1.0)

# Global CMake variables are set here We use C++20, with the assumption that we
# only implement features that are supported by GCC, Clang, MSVC, Apple Clang
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use extensions, because they might affect compiler compatibility
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  geodepot-api
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)
FetchContent_MakeAvailable(googletest Catch2)


geodepotinit("my-url")
geodepotget("wippolder/wippolder.gpkg")

add_executable("test_geodepot_cmake" "${CMAKE_CURRENT_SOURCE_DIR}/test_geodepot_cmake.cpp")

add_test(
  NAME "function-using-geodepot-data"
  COMMAND test_geodepot_cmake "${GEODEPOT_CASES}/wippolder/wippolder.gpkg"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
