add_library(geodepot geodepot.cpp)
target_include_directories(geodepot PUBLIC "${GEODEPOT_INCLUDE_DIR}")
target_link_libraries(geodepot PRIVATE CURL::libcurl nlohmann_json::nlohmann_json ${GEODEPOT_ARCHIVE_TARGET} OpenSSL::Crypto)
target_compile_features(geodepot PUBLIC cxx_std_20)
