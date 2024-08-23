set(LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/geodepot-py.cpp")
set(GEODEPOT_LINK_LIBS geodepot)

find_package(
  Python 3.12
  COMPONENTS Interpreter Development.Module
  REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

python_add_library(_core MODULE ${LIB_SOURCES} WITH_SOABI)
target_link_libraries(_core PRIVATE ${GEODEPOT_LINK_LIBS} pybind11::headers)

target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS _core LIBRARY DESTINATION geodepot)
