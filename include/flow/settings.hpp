#pragma once

namespace flow {
template<units::Unit Unit, units::Representation Rep>
struct settings {
  units::isq::si::frequency<Unit, Rep> frequency{};
};

template<typename settings_t>
concept is_settings = requires(settings_t settings)
{
  settings.frequency;
};

template<units::Unit Unit, units::Representation Rep>
auto make_settings(units::isq::si::frequency<Unit, Rep> frequency)
{
  return settings<Unit, Rep>{ frequency };
}

}// namespace flow