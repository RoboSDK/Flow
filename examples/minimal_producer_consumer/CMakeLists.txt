list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(minimal_producer_consumer minimal_producer_consumer.cpp)
target_link_libraries(minimal_producer_consumer PRIVATE ${libraries})
