list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(minimal_publisher_subscriber minimal_publisher_subscriber.cpp)
target_link_libraries(minimal_publisher_subscriber PRIVATE ${libraries})
