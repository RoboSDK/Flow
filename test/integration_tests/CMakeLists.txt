list(APPEND libraries CONAN_PKG::catch2 project_options project_warnings header_only_libs flow::flow cppcoro liburing::liburing)

macro(add_coroutine_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE ${libraries})
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

add_coroutine_test(test_channel)
add_coroutine_test(test_registry)
add_coroutine_test(test_single_producer_single_consumer)
#add_coroutine_test(test_multi_producer_multi_consumer)
#add_coroutine_test(test_single_producer_multi_consumer)
#add_coroutine_test(test_multi_producer_single_consumer)
