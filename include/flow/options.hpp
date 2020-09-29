#pragma once

namespace flow {

template <std::size_t bytes = 512>
struct linker_buffer_size {};

template<std::size_t linker_buffer_size_bytes>
struct options {
  static constexpr auto linker_buffer_size = linker_buffer_size_bytes;
};

template <typename options_t>
concept options_concept = requires(options_t options) {
  options.linker_buffer_size;
};

template <std::size_t linker_buffer_size_bytes>
consteval auto make_options(linker_buffer_size<linker_buffer_size_bytes> /*unused*/)
{
  return options<linker_buffer_size_bytes>{};
}

}// namespace flow
