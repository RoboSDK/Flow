add_library(utility INTERFACE)
add_library(flow::utility ALIAS utility)


target_include_directories(utility INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(utility
        INTERFACE
        project_options
        project_warnings
        header_only_libs)
