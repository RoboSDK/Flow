add_library(flow INTERFACE)
target_include_directories(flow INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
        flow
        INTERFACE
        project_options
        project_warnings
        header_only_libs
        CONAN_PKG::spdlog
        CONAN_PKG::docopt.cpp)
