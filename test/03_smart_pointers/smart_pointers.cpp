#include "smart_pointers.h"

namespace chimera_test {
namespace nested_namespace {

//==============================================================================
std::unique_ptr<Example> create_example()
{
  return std::unique_ptr<Example>(new Example());
}

//==============================================================================
std::shared_ptr<ExampleShared> create_example_shared()
{
  return std::shared_ptr<ExampleShared>(new ExampleShared());
}

} // namespace nested_namespace
} // namespace chimera_test
