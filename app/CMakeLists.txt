# Generic test that uses conan libs
add_executable(intro main.cpp)
target_link_libraries(
  intro
  PRIVATE project_options
          project_warnings
          header_only_libs
          flow

          CONAN_PKG::spdlog)
