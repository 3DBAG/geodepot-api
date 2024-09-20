cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-integration-tests)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(CPM)

CPMAddPackage("gh:3dbag/geodepot-api")

#include(FetchContent)
#FetchContent_Declare(
#  geodepot-api
#  GIT_REPOSITORY https://github.com/3DBAG/geodepot-api.git
#)
#FetchContent_MakeAvailable(geodepot-api)

add_executable("test_integration" "test_integration.cpp")
target_link_libraries("test_integration" geodepot)