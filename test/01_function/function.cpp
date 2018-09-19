#include "function.h"

namespace function {
namespace nested_function {

//==============================================================================
int add(int i, int j)
{
  return i + j;
}

//==============================================================================
void void_pointer_param(void*)
{
  // Do nothing
}

} // namespace nested_function
} // namespace function
