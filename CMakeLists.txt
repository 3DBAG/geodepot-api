cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-api VERSION 0.1.0)

option(GD_BUILD_APPS "Build applications in addition to library" ON)
option(GD_BUILD_TESTING "Enable tests for geodepot" OFF)
option(GD_BUILD_BINDINGS "Build python bindings with pybind" OFF)
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)
# Set variables for building the bindings
if(GD_BUILD_BINDINGS)
  message(
    STATUS
      "Configuring static library for Geodepot because GD_BUILD_BINDINGS is ON")
  set(BUILD_SHARED_LIBS OFF)
endif()
if(NOT BUILD_SHARED_LIBS)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
endif()

# Global CMake variables are set here We use C++20, with the assumption that we
# only implement features that are supported by GCC, Clang, MSVC, Apple Clang
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use extensions, because they might affect compiler compatibility
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake modules, like the documentation module, go in here
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# EXTERNAL LIBRARIES
include(CPM)
find_package(CURL REQUIRED)
cpmaddpackage("gh:libarchive/libarchive@3.7.4")
cpmaddpackage(
  NAME
  libarchive
  VERSION
  3.7.4
  GITHUB_REPOSITORY
  libarchive/libarchive
  OPTIONS
  "ENABLE_TAR ON"
  "ENABLE_TAR_SHARED ON"
  "ENABLE_TEST OFF"
  "ENABLE_CPIO OFF"
  "ENABLE_XATTR OFF"
  "ENABLE_ACL OFF")

# PROJECT LIBRARY
set(GEODEPOT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_subdirectory(src)

# PROJECT EXECUTABLES
if(GD_BUILD_APPS)
  add_subdirectory(apps)
endif()

# PROJECT PYTHON BINDINGS
if(GD_BUILD_BINDINGS)
  add_subdirectory(geodepot-py)
  # add_subdirectory(geodepot-cmake)
endif()

# PROJECT TESTS
if(GD_BUILD_TESTING)
  # Must enable_testing() before add_subdirectory(tests)
  enable_testing()
  add_subdirectory(tests)
endif()
