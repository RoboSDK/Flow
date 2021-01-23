list(APPEND libraries
        CONAN_PKG::catch2
        project_options
        project_warnings
        header_only_libs
        flow::flow
        flow::logging cppcoro
        liburing::liburing
        CONAN_PKG::spdlog)

macro(add_coroutine_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE ${libraries})
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

add_coroutine_test(test_chain_function_pointers)

if (ENABLE_EXAMPLES)
    list(APPEND examples
                minimal_producer_consumer
                simple_producer_consumer
                simple_user_producer_consumer
                producer_transformer_consumer)

    foreach (example ${examples})
        add_test(NAME ${example} COMMAND ${example})
    endforeach ()
endif ()
