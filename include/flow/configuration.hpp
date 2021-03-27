#pragma once

#include <chrono>

#include "flow/literals.hpp"

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
  static constexpr std::size_t message_buffer_size = 128;
  static constexpr std::size_t stride_length = 32;

  static constexpr units::physical::Frequency auto frequency =
    units::physical::si::frequency<units::physical::si::hertz>(10);
};
}// namespace flow
