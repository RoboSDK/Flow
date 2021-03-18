list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(simple_user_publisher_subscriber  simple_user_publisher_subscriber.cpp)
target_link_libraries(simple_user_publisher_subscriber PRIVATE ${libraries})
