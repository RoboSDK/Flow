#ifndef FLOW_FLOW_HPP
#define FLOW_FLOW_HPP

#include <cppitertools/enumerate.hpp>
#include <iostream>
#include <docopt/docopt.h>

namespace flow {
inline void begin(int argc, const char **argv)
{
  static constexpr auto USAGE =
    R"(Naval Fate.

     Usage:
          naval_fate ship new <name>...
          naval_fate ship <name> move <x> <y> [--speed=<kn>]
          naval_fate ship shoot <x> <y>
          naval_fate mine (set|remove) <x> <y> [--moored | --drifting]
          naval_fate (-h | --help)
          naval_fate --version

     Options:
              -h --help     Show this screen.
              --version     Show version.
              --speed=<kn>  Speed in knots [default: 10].
              --moored      Moored (anchored) mine.
              --drifting    Drifting mine.
    )";

  static auto constexpr SHOW_HELP_IF_REQUESTED = true;
  static auto constexpr VERSION_INFO = "Naval Fate 2.0";

  auto const argument_values = std::vector<std::string>{ std::next(argv), std::next(argv, argc) };
  auto const arguments = docopt::docopt(USAGE, argument_values, SHOW_HELP_IF_REQUESTED, VERSION_INFO);

  for (auto const&[argument, value] : arguments) {
    std::cout << "{" << argument << ", " << value << "}";
  }
}
}// namespace flow

#endif//FLOW_FLOW_HPP
