include(GeoDepot.cmake)

# fetchcontent download and compile the exe
# use reference the compiled exe

function(GeodepotInit URL)
    execute_process(
            COMMAND $<TARGET_FILE:geodepot-init>
            OUTPUT_VARIABLE GEODEPOT_INIT_RESULT
    )
endfunction()

function(GeodepotGet CASESPEC)
    execute_process(
            COMMAND $<TARGET_FILE:geodepot-get>
            OUTPUT_VARIABLE GEODEPOT_GET_RESULT
    )
endfunction()

if (GD_BUILD_TESTING)
    add_subdirectory(tests)
endif ()