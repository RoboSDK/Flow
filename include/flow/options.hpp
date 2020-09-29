#pragma once

namespace flow {

template<class T>
concept options_concept= std::is_same_v<T, flow::options>;

template<std::size_t linker_buffer_size_t = 64>
struct options {
  static constexpr auto linker_buffer_size = linker_buffer_size_t;
};
}// namespace flow
