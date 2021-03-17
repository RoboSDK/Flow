#pragma once

/**
 * Global compile time configuration that determine channel buffer sizes and among other
 * options.
 *
 * This file may be copied and modified and passed in like so
 *
 * #include <my_configuration.hpp>
 *
 * int main() {
 *  auto network = flow::network<my::configuration>(...);
 * }
 */
namespace flow {
struct configuration {
  static constexpr std::size_t max_resources = 256;
  static constexpr std::size_t message_buffer_size = 32;
  static constexpr std::size_t stride_length = 2;
};
}// namespace flow
