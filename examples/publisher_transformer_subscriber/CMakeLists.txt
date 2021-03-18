list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(publisher_transformer_subscriber publisher_transformer_subscriber.cpp)
target_link_libraries(publisher_transformer_subscriber PRIVATE ${libraries})
