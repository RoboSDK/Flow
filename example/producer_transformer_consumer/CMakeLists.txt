list(APPEND libraries
        project_options
        project_warnings
        header_only_libs

        flow::flow
        flow::logging cppcoro
        liburing::liburing
        CONAN_PKG::spdlog)

add_executable(producer_transformer_consumer main.cpp)
target_link_libraries(producer_transformer_consumer PRIVATE ${libraries})
