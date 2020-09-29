#pragma once

namespace flow {

template<std::size_t linker_buffer_size_t = 64>
struct options {
  static constexpr auto linker_buffer_size = linker_buffer_size_t;
};

}// namespace flow
