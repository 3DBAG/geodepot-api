cmake_minimum_required(VERSION 3.25)
project(geodepot_api)

# Options
option(GD_BUILD_APPS "Build applications in addition to library" ON)
option(GD_BUILD_TESTING "Enable tests for geodepot" ON)

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

# PROJECT LIBRARY
set(GEODEPOT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
add_subdirectory(src)

# PROJECT EXECUTABLES
if(GD_BUILD_APPS)
  add_subdirectory(apps)
endif()

# PROJECT TESTS
if(GD_BUILD_TESTING)
  # Must enable_testing() before add_subdirectory(tests)
  enable_testing()
  add_subdirectory(tests)
endif()
