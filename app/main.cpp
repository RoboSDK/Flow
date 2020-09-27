#include <flow/flow.hpp>
#include <flow/logging.hpp>

struct Localization {};
struct Filtering {};
struct Sensor {};

struct Transforms {};
struct Behaviors {};
struct Planning {};
struct Controls {};

int main(int argc, const char** argv) {
  flow::begin(argc, argv);
  flow::logging::info("Hello, World");

//  [[maybe_unused]] constexpr auto autonomous_system = flow::make_system<Sensor, Filtering, Localization, Transforms, Behaviors, Planning, Controls>();
  //  return flow::spin(autonomous_system);
//  boost::sml::event
}
