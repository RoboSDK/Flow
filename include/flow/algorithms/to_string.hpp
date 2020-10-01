#pragma once

#include <sstream>

namespace flow {
template<typename begin_t, typename end_t>
std::string to_string(begin_t begin, end_t end, std::string delim = ",")
{
  std::stringstream ss;
  ss << "{";
  std::string d{};
  for (; begin != end; ++begin) {
    ss << d << std::to_string(*begin);
    d = delim + " ";
  }
  ss << "}\n";
  return ss.str();
}
}// namespace flow