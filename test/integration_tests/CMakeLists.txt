list(APPEND libraries CONAN_PKG::catch2 project_options project_warnings header_only_libs flow::flow cppcoro liburing::liburing)

macro(add_coroutine_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE ${libraries})
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

add_coroutine_test(test_channel)
add_coroutine_test(test_registry)
add_coroutine_test(test_1_msg_1_pub_1_sub)
add_coroutine_test(test_1_msg_1_pub_1_sub_cancel_delayed)
