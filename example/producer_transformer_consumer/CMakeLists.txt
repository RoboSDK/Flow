list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(producer_transformer_consumer producer_transformer_consumer.cpp)
target_link_libraries(producer_transformer_consumer PRIVATE ${libraries})
