cmake_minimum_required(VERSION 3.19...3.29)
project(geodepot-integration-tests)

include(FetchContent)
FetchContent_Declare(
  geodepot-api
  GIT_REPOSITORY https://github.com/3DBAG/geodepot-api.git
)
FetchContent_MakeAvailable(geodepot-api)

add_executable("test_integration" "test_integration.cpp")
target_link_libraries("test_integration" geodepot)