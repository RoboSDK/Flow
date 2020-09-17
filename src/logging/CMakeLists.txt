add_library(logging INTERFACE)
add_library(flow::logging ALIAS logging)


# TODO: Make these implementations non-template, or figure out a way to hide spdlog from the user
target_include_directories(logging INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(logging
        INTERFACE
        project_options
        project_warnings
        header_only_libs

        CONAN_PKG::spdlog)
