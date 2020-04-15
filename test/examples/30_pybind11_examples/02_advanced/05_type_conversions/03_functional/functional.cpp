#include "functional.h"

namespace chimera_test
{

//==============================================================================
int func_arg(const std::function<int(int)> &f)
{
    return f(10);
}

//==============================================================================
std::function<int(int)> func_ret(const std::function<int(int)> &f)
{
    return [f](int i) { return f(i) + 1; };
}

//==============================================================================
pybind11::cpp_function func_cpp()
{
    return pybind11::cpp_function([](int i) { return i + 1; },
                                  pybind11::arg("number"));
}

} // namespace chimera_test
