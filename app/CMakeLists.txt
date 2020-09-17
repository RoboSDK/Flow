add_executable(app main.cpp ../include/flow/perception/PerceptionTask.hpp ../include/flow/perception/PerceptionLayer.hpp ../include/flow/executive/ExecutiveTask.hpp ../include/flow/executive/ExecutiveLayer.hpp)
target_link_libraries(app
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::logging
        flow::flow
)
