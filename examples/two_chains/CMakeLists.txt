list(APPEND libraries flow::flow CONAN_PKG::spdlog)

add_executable(two_chains two_chains.cpp)
target_link_libraries(two_chains PRIVATE ${libraries})
