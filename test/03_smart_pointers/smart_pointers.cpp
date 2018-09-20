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

//==============================================================================
int void_pointer_param(void* dummy)
{
  Dummy* casted = static_cast<Dummy*>(dummy);
  return casted->val;
}

//==============================================================================
void void_param()
{
  // Do nothing
}

//==============================================================================
void void_return()
{
  // Do nothing
}

} // namespace nested_namespace
} // namespace chimera_test
