#include "function.h"

namespace chimera_test
{

//==============================================================================
int add(int i, int j)
{
    return i + j;
}

//==============================================================================
int void_pointer_param(void *dummy)
{
    Dummy *casted = static_cast<Dummy *>(dummy);
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

//==============================================================================
void function_with_suppressed_param(const SuppressedClass &)
{
    // Do nothing
}

//==============================================================================
void function_with_suppressed_template_param(
    const SuppressedTemplateClass<int> &)
{
    // Do nothing
}

} // namespace chimera_test
