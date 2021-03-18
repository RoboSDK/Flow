list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(publisher_transformer_consumer publisher_transformer_consumer.cpp)
target_link_libraries(publisher_transformer_consumer PRIVATE ${libraries})
