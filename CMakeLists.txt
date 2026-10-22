cmake_minimum_required(VERSION 3.15)
project(Nome6UGens LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT SC_PATH)
    message(FATAL_ERROR
        "Set SC_PATH to the SuperCollider source tree, e.g. -DSC_PATH=C:/src/supercollider")
endif()

add_library(Nome6 MODULE
    source/Nome6.cpp
)

target_include_directories(Nome6 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${SC_PATH}/include/plugin_interface
    ${SC_PATH}/include/common
    ${SC_PATH}/common
)

if(MSVC)
    target_compile_options(Nome6 PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/fp:fast>
    )
else()
    target_compile_options(Nome6 PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
    )
endif()

set_target_properties(Nome6 PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Keep the same extension convention used by the other custom UGens in this
# project: .scx on Windows/macOS and .so on Linux.
if(APPLE OR WIN32)
    set_target_properties(Nome6 PROPERTIES SUFFIX ".scx")
endif()

install(TARGETS Nome6
    LIBRARY DESTINATION Nome6/plugins
    RUNTIME DESTINATION Nome6/plugins
)
install(DIRECTORY Classes DESTINATION Nome6)
install(DIRECTORY HelpSource DESTINATION Nome6)
install(DIRECTORY examples DESTINATION Nome6)
install(FILES README.md DESTINATION Nome6)

include(CTest)
if(BUILD_TESTING)
    foreach(test_name core family oversampling noise benchmark)
        add_executable(nome6_${test_name}_test tests/${test_name}_test.cpp)
        target_include_directories(nome6_${test_name}_test PRIVATE source)
        if(MSVC)
            target_compile_options(nome6_${test_name}_test PRIVATE
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:Release>:/fp:fast>
            )
        else()
            target_compile_options(nome6_${test_name}_test PRIVATE
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:Release>:-ffast-math>
            )
        endif()
    endforeach()
    add_test(NAME nome6_core_test COMMAND nome6_core_test)
    add_test(NAME nome6_family_test COMMAND nome6_family_test)
    add_test(NAME nome6_oversampling_test COMMAND nome6_oversampling_test)
    add_test(NAME nome6_noise_test COMMAND nome6_noise_test)
endif()
