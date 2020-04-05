#pragma once

#include <memory>

namespace chimera_test
{

class A
{
};

inline void void_bool(bool)
{
}
inline void void_int(int)
{
}
inline void void_long(long)
{
}
inline void void_float(float)
{
}
inline void void_double(double)
{
}
inline void void_a(A)
{
}
inline void void_shared_a(std::shared_ptr<A>)
{
}

int add(int i = 1, int j = 2);

inline int inline_add(int i = 1, int j = 2)
{
    return i + j;
}

struct Dummy
{
    int val;
    Dummy() = default;
};

int void_pointer_param(void *dummy);

void void_param(void);

void void_return();

// This class is suppressed by the configuration so that not to be binded
class SuppressedClass
{
};

// This function should be suppressed as well because the parameter is of a
// suppressed type.
void function_with_suppressed_param(const SuppressedClass &);

// This class is suppressed by the configuration so that not to be binded
template <typename T>
class SuppressedTemplateClass
{
};

// This function should be suppressed as well because the parameter is of a
// suppressed type.
void function_with_suppressed_template_param(
    const SuppressedTemplateClass<int> &);

} // namespace chimera_test
