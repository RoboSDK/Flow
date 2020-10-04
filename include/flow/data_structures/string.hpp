#pragma once

#include <iomanip>
#include <sstream>

#include <frozen/string.h>

namespace flow {
using string = frozen::string;

/**
 * Takes a begin and end iterator for some container and turns it into a string
 * @param begin_t The begin iterator
 * @param end_t The end iterator
 * @param delim The delimeter that separates the strings
 * @return A string representing the list
 */
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
  ss << "}";
  return ss.str();
}
}// namespace flow
