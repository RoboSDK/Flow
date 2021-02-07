list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(simple_user_producer_consumer  simple_user_producer_consumer.cpp)
target_link_libraries(simple_user_producer_consumer PRIVATE ${libraries})
