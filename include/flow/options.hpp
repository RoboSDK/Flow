#pragma once

namespace flow {

template<std::size_t bytes = 64>
struct scheduler_subscriber_buffer_size {
  static constexpr auto size = bytes;
};

template<std::size_t bytes = 64>
struct scheduler_publisher_buffer_size {
  static constexpr auto size = bytes;
};

template<
  std::size_t scheduler_subscriber_buffer_size_bytes,
  std::size_t scheduler_publisher_buffer_size_bytes>
struct options {
  static constexpr std::size_t scheduler_subscriber_buffer_size = scheduler_subscriber_buffer_size_bytes;
  static constexpr std::size_t scheduler_publisher_buffer_size = scheduler_publisher_buffer_size_bytes;
};

template<typename options_t>
concept options_concept = requires(options_t options)
{
  options.scheduler_subscriber_buffer_size;
  options.scheduler_publisher_buffer_size;
};

consteval auto make_options()
{
  constexpr auto subscriber_size = scheduler_publisher_buffer_size{};
  constexpr auto publisher_size = scheduler_publisher_buffer_size{};
  return options<subscriber_size.size, publisher_size.size>{};
}

template<
  std::size_t scheduler_subscriber_buffer_size_bytes,
  std::size_t scheduler_publisher_buffer_size_bytes>
consteval auto make_options(
  scheduler_subscriber_buffer_size<scheduler_subscriber_buffer_size_bytes> /*unused*/,
  scheduler_publisher_buffer_size<scheduler_publisher_buffer_size_bytes> /*unused*/)
{
  return options<scheduler_subscriber_buffer_size_bytes, scheduler_publisher_buffer_size_bytes>{};
}

}// namespace flow
