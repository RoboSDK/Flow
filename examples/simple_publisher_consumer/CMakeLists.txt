list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(simple_publisher_consumer  simple_publisher_consumer.cpp)
target_link_libraries(simple_publisher_consumer PRIVATE ${libraries})
