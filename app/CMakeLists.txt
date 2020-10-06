add_executable(app main.cpp LidarTask.hpp)
target_link_libraries(app
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        cppcoro
        flow::flow
)
