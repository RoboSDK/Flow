#pragma once

#include <cppcoro/task.hpp>
#include <units/chrono.h>
#include <units/isq/si/si.h>

namespace flow {
constexpr std::chrono::nanoseconds period_in_nanoseconds(units::isq::Frequency auto frequency)
{
  using namespace units;
  double period_in_nanoseconds = quantity_cast<isq::si::nanosecond>(1.0 / frequency).number();
  return std::chrono::nanoseconds{ static_cast<long int>(period_in_nanoseconds) };
}
}// namespace flow