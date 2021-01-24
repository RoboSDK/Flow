#pragma once

#include <string>

namespace flow {
struct options {
  std::string publish_to{};
  std::string subscribe_to{};
};
}