#pragma once

#include <functional>
#include <string>

#include <pybind11/pybind11.h>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/advanced/cast/functional.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//-------------------------------------------
// Callbacks and passing anonymous functions
//-------------------------------------------

int func_arg(const std::function<int(int)> &f);

std::function<int(int)> func_ret(const std::function<int(int)> &f);

pybind11::cpp_function func_cpp();

//---------------------------------------------
// TODO: Add more sections...
//---------------------------------------------

} // namespace chimera_test
