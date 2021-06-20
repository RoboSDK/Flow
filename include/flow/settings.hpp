#pragma once

#include <optional>

namespace flow {
struct settings {
  std::optional<std::chrono::nanoseconds> period{std::nullopt};
};

template<typename settings_t>
concept is_settings = std::is_same_v<settings, std::decay_t<settings_t>>;

constexpr auto make_settings(std::chrono::nanoseconds period)
{
  return settings{ period };
};

}// namespace flow