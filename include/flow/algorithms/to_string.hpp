#pragma once

#include <sstream>

namespace flow {
template<typename iterator_t, typename callback_t>
std::string to_string(iterator_t begin, iterator_t end, char delim, callback_t to_string_item)
{
  std::stringstream ss;
  ss << "{";
  std::string d{};
  for (; begin != end; ++begin) {
    ss << d << to_string_item(*begin);
    d = std::string(1, delim) + " ";
  }
  ss << "}\n";
  return ss.str();
}
}// namespace flow