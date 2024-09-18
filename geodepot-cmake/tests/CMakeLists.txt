cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-cmake-test VERSION 0.1.0)

# Global CMake variables are set here We use C++20, with the assumption that we
# only implement features that are supported by GCC, Clang, MSVC, Apple Clang
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use extensions, because they might affect compiler compatibility
set(CMAKE_CXX_EXTENSIONS OFF)

# -----
include(FetchContent)
set(GD_BUILD_TESTING OFF)
set(GD_BUILD_BINDINGS OFF)
set(GD_BUILD_APPS ON)
FetchContent_Declare(
  geodepot-api
  /home/balazs/Development/geodepot-api
)
FetchContent_MakeAvailable(geodepot-api)

function(GeodepotInit URL)
    message(${URL})
    execute_process(
            COMMAND geodepot-init ${URL}
            OUTPUT_VARIABLE GEODEPOT_INIT_RESULT
    )
endfunction()

function(GeodepotGet CASESPEC)
    message(${CASESPEC})
    execute_process(
            COMMAND geodepot-get ${CASESPEC}
            OUTPUT_VARIABLE GEODEPOT_GET_RESULT
    )
endfunction()

# -----

GeodepotInit("https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot")
GeodepotGet("wippolder/wippolder.gpkg")

message(STATUS ${GEODEPOT_INIT_RESULT})
message(STATUS ${GEODEPOT_GET_RESULT})

add_executable("test_geodepot_cmake" "test_geodepot_cmake.cpp")

add_test(
  NAME "function-using-geodepot-data"
  COMMAND test_geodepot_cmake "${GEODEPOT_CASES}/wippolder/wippolder.gpkg"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
