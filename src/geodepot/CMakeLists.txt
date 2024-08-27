set(LIBRARY_SOURCES "geodepot.cpp")
set(LIBRARY_HEADERS "${GEODEPOT_INCLUDE_DIR}/geodepot/geodepot.h")
set(LIBRARY_INCLUDES "${GEODEPOT_INCLUDE_DIR}")

add_library("geodepot" SHARED ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
target_link_libraries("geodepot" CURL::libcurl zip)
target_include_directories("geodepot" PUBLIC ${LIBRARY_INCLUDES})