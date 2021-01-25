list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(simple_producer_consumer  simple_producer_consumer.cpp)
target_link_libraries(simple_producer_consumer PRIVATE ${libraries})
