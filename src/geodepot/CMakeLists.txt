set(LIBRARY_SOURCES "geodepot.cpp")
set(LIBRARY_HEADERS "${GEODEPOT_INCLUDE_DIR}/geodepot/geodepot.h")
set(LIBRARY_INCLUDES "${GEODEPOT_INCLUDE_DIR}")

add_library("geodepot" ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
target_link_libraries("geodepot" PRIVATE CURL::libcurl nlohmann_json::nlohmann_json archive_static)
target_include_directories("geodepot" PUBLIC ${LIBRARY_INCLUDES})
if(${GD_BUILD_BINDINGS})
  # The Python bindings require -fPIC for the geodepot library
  set_target_properties("geodepot" PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
  set_target_properties("geodepot" PROPERTIES POSITION_INDEPENDENT_CODE
                                              ${BUILD_SHARED_LIBS})
endif()
