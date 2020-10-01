#include <flow/system.hpp>
#include <flow/registry.hpp>

#include "LidarData.hpp"
#include "SensorLayer.hpp"
#include "TransformLayer.hpp"

int main()
{
  using namespace app;
  auto registry = flow::make_registry<LidarData>();
  auto system = flow::make_system<TransformLayer, SensorLayer>();
  flow::spin(system, registry);
}
