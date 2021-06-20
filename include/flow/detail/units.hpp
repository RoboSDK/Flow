#pragma once

#include <cppcoro/task.hpp>
#include <units/chrono.h>
#include <units/isq/si/si.h>

namespace flow {
std::chrono::nanoseconds period_in_nanoseconds(units::isq::Frequency auto frequency) {
  return std::chrono::nanoseconds{units::quantity_cast<units::isq::si::nanosecond>(1 / frequency).number()};
}
}