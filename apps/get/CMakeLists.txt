add_executable("geodepot-get" "get.cpp")
set_target_properties("geodepot-get" PROPERTIES CXX_STANDARD 20)
target_link_libraries("geodepot-get" PRIVATE geodepot)