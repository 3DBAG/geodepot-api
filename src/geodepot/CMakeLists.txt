set(LIBRARY_SOURCES "geodepot.cpp")
set(LIBRARY_HEADERS "${GEODEPOT_INCLUDE_DIR}/geodepot/geodepot.h")
set(LIBRARY_INCLUDES "${GEODEPOT_INCLUDE_DIR}")

add_library("geodepot" ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
target_link_libraries("geodepot" PRIVATE CURL::libcurl)
target_include_directories("geodepot" PUBLIC ${LIBRARY_INCLUDES})
set_target_properties("geodepot" PROPERTIES POSITION_INDEPENDENT_CODE ON)
