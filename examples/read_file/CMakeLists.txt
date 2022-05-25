list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(read_file read_file.cpp)
target_link_libraries(read_file PRIVATE ${libraries})
