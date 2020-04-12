#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/basics.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//-----------------------------------------
// Creating bindings for a simple function
//-----------------------------------------

int add(int i, int j);

//-------------------
// Keyword arguments
//-------------------

// Keyword argument is supported by default

//-------------------
// Default arguments
//-------------------

int add_def_args(int i = 1, int j = 2);

} // namespace chimera_test
