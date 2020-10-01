#pragma once

namespace flow {

template<std::size_t bytes = 64>
struct pub_sub_buffer_size {
  static constexpr auto size = bytes;
};

template<
  std::size_t pub_sub_buffer_size_bytes>
struct options {
  static constexpr std::size_t pub_sub_buffer_size = pub_sub_buffer_size_bytes;
};

template<typename options_t>
concept options_concept = requires(options_t options)
{
  options.pub_sub_buffer_size;
};

consteval auto make_options()
{
  return options<pub_sub_buffer_size{}.size>{};
}

template< std::size_t pub_sub_buffer_size_bytes>
consteval auto make_options(pub_sub_buffer_size<pub_sub_buffer_size_bytes> /*unused*/)
{
  return options<pub_sub_buffer_size_bytes>{};
}

}// namespace flow
