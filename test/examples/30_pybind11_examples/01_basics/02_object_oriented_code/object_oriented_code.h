#pragma once

#include <string>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/classes.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//------------------------------------------------------------------
// Creating bindings for a custom type
// & Instance and static fields
// & Dynamic attributes
//------------------------------------------------------------------

/// Pet implementation that has name
struct Pet
{
    Pet(const std::string &name) : name(name)
    {
    }
    void setName(const std::string &name_)
    {
        name = name_;
    }
    const std::string &getName() const
    {
        return name;
    }

    std::string name;
};

} // namespace chimera_test
