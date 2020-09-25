#include <flow/flow.hpp>
#include <flow/logging.hpp>
#include <flow/AutonomousSystem.hpp>

struct Localization {};
struct Filtering {};
struct Sensor {};

struct Transforms {};
struct Behaviors {};
struct Planning {};
struct Controls {};

int main(int argc, const char **argv) {
  flow::begin(argc, argv);
  flow::logging::info("Hello, World");

  constexpr auto perception_system = flow::make_system<Localization, Filtering, Sensor>();
  constexpr auto executive_system = flow::make_system<Transforms, Behaviors, Planning, Controls>();
  [[maybe_unused]] constexpr auto autonomous_system = flow::make_autonomous_system(perception_system, executive_system);
  //  return flow::spin(autonomous_system);
}
