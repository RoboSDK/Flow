list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(minimal_publisher_consumer minimal_publisher_consumer.cpp)
target_link_libraries(minimal_publisher_consumer PRIVATE ${libraries})
