#include "eigen.h"

namespace chimera_test {
namespace nested_namespace {

//==============================================================================
void function_with_map_param(Eigen::Map<Eigen::VectorXd> map)
{
  std::cout << "size: " << map.size() << std::endl;
}

//==============================================================================
void test_function_with_map_param()
{
  Eigen::VectorXd vec = Eigen::VectorXd::Zero(10);
  Eigen::Map<Eigen::VectorXd> map(vec.data(), vec.size());
  function_with_map_param(map);
}

} // namespace nested_namespace
} // namespace chimera_test
