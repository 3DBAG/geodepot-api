find_package(Python3 3.12 COMPONENTS Interpreter REQUIRED)

add_executable(geodepot-api-probe test_integration.cpp)
target_link_libraries(geodepot-api-probe PRIVATE geodepot)

add_test(
  NAME geodepot-remote-integration
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_remote.py"
          --get "$<TARGET_FILE:geodepot-get>"
          --probe "$<TARGET_FILE:geodepot-api-probe>"
          --source-root "${CMAKE_SOURCE_DIR}")
set_tests_properties(geodepot-remote-integration PROPERTIES LABELS integration TIMEOUT 600)

add_custom_target(test-integration
  COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -L integration
  DEPENDS geodepot-get geodepot-api-probe
  USES_TERMINAL)
