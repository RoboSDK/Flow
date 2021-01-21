list(APPEND libraries
        project_options
        project_warnings
        header_only_libs

        flow::flow
        flow::logging cppcoro
        liburing::liburing
        CONAN_PKG::spdlog)

add_executable(simple_producer_consumer main.cpp)
target_link_libraries(simple_producer_consumer PRIVATE ${libraries})
