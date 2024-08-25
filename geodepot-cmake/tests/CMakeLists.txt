cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-cmake-test VERSION 0.1.0)

# Global CMake variables are set here We use C++20, with the assumption that we
# only implement features that are supported by GCC, Clang, MSVC, Apple Clang
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use extensions, because they might affect compiler compatibility
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(GD_BUILD_TESTING OFF)
set(GD_BUILD_BINDINGS OFF)
set(GD_BUILD_APPS ON)
FetchContent_Declare(
  geodepot-api
  GIT_REPOSITORY git@github.com:3DGI/geodepot-api.git
  GIT_TAG        d28306a4742a649ed38e26f7b192f63b7dc8a9e1
)
FetchContent_MakeAvailable(geodepot-api)

add_executable("test_geodepot_cmake" "test_geodepot_cmake.cpp")

add_test(
  NAME "function-using-geodepot-data"
  COMMAND test_geodepot_cmake "${GEODEPOT_CASES}/wippolder/wippolder.gpkg"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
