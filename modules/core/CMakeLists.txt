add_library(flow STATIC flow.cpp)
add_library(flow::flow ALIAS flow)

target_include_directories(flow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(flow
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::logging

        CONAN_PKG::docopt.cpp
        )
