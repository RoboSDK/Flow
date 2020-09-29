list(APPEND source_files flow.cpp)

if (BUILD_SHARED_LIBS)
    add_library(flow SHARED ${source_files})
else()
    add_library(flow STATIC ${source_files})
endif()

add_library(flow::flow ALIAS flow)

target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(flow
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::logging

        CONAN_PKG::docopt.cpp)
