#pragma once

#include <exception>
#include <stdexcept>

#include <pybind11/pybind11.h>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//----------------------------------------
// Overriding virtual functions in Python
//----------------------------------------

inline void throw_std_exception()
{
    throw std::exception();
}

inline void throw_bad_alloc()
{
    throw std::bad_alloc();
}

inline void throw_domain_error()
{
    throw std::domain_error("");
}

inline void throw_invalid_argument()
{
    throw std::invalid_argument("");
}

inline void throw_length_error()
{
    throw std::length_error("");
}

inline void throw_out_of_range()
{
    throw std::out_of_range("");
}

inline void throw_range_error()
{
    throw std::range_error("");
}

inline void throw_overflow_error()
{
    throw std::overflow_error("");
}

//---------------------------------------------
// Registering custom translators
//---------------------------------------------

// TODO: Add tests

} // namespace chimera_test
