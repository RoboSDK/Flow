#pragma once

#include <cppcoro/task.hpp>
#include <units/chrono.h>

namespace flow {
std::chrono::nanoseconds period_in_nanoseconds(units::isq::Frequency auto frequency) {
  return units::quantity_cast<units::isq::si::nanosecond>(1 / frequency).number();
}
}