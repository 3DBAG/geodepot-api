set(DATA_URL_ROOT "https://data.3dgi.xyz/geodepot-test-data")
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

cpmaddpackage("gh:catchorg/Catch2@3.4.0")
include(FetchContent)

FetchContent_Declare(
  geodepot_data
  URL "${DATA_URL_ROOT}/data.zip"
  URL_HASH MD5=22b765db5a1dae2a1a7fed41958beb3d
  SOURCE_DIR "${DATA_DIR}")
FetchContent_MakeAvailable(geodepot_data)

# --- Library testing
add_executable("test_geodepot" "${CMAKE_CURRENT_SOURCE_DIR}/test_geodepot.cpp")
target_link_libraries("test_geodepot" PRIVATE geodepot Catch2::Catch2WithMain)

# --- Integration tests that are run on the built executables, before
# installation.
if(GD_BUILD_APPS)
  add_test(
    NAME "get"
    COMMAND $<TARGET_FILE:geodepot-get>
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_test(
    NAME "init"
    COMMAND $<TARGET_FILE:geodepot-init>
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
