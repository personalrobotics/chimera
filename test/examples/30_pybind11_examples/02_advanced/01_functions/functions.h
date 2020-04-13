#pragma once

#include <iostream>
#include <string>

#include <pybind11/pybind11.h>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/advanced/functions.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//-----------------------
// Return value policies
//-----------------------

// TODO: Add examples

//--------------------------
// Additional call policies
//--------------------------

// TODO: Add examples

//-----------------------------
// Python objects as arguments
//-----------------------------

// TODO: Fix not able to generate function with arguments passed by value
void print_dict(const pybind11::dict &dict);

//------------------------------
// Accepting *args and **kwargs
//------------------------------

// TODO: Add examples

//-----------------------------
// Default arguments revisited
//-----------------------------

// TODO: Add examples

//--------------------------
// Non-converting arguments
//--------------------------

// TODO: Add examples

//----------------------------------
// Allow/Prohibiting None arguments
//----------------------------------

// TODO: Add examples

//---------------------------
// Overload resolution order
//---------------------------

// TODO: Add examples

} // namespace chimera_test
