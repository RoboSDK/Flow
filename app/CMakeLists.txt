add_executable(app main.cpp)
target_link_libraries(
        app
        PRIVATE project_options
        project_warnings
        header_only_libs

        flow::flow
        flow::logging
)
