#include "function.h"

namespace function {
namespace nested_function {

//==============================================================================
int add(int i, int j)
{
  return i + j;
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

} // namespace nested_function
} // namespace function
