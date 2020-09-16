add_library(executive STATIC ExecutiveSystem.cpp)
add_library(flow::executive ALIAS executive)

target_include_directories(executive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(executive
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::logging

        CONAN_PKG::docopt.cpp
        )
