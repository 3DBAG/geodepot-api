add_executable("geodepot-init" "init.cpp")
set_target_properties("geodepot-init" PROPERTIES CXX_STANDARD 20)
target_link_libraries("geodepot-init" PRIVATE geodepot)