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

//----------------------------------------
// Overriding virtual functions in Python
//----------------------------------------

class Animal
{
public:
    virtual ~Animal()
    {
    }
    virtual std::string go(int n_times) = 0;
    virtual void virtual_foo()
    {
    }
    void foo()
    {
    }
};

class Dog : public Animal
{
public:
    Dog() = default; // TODO: Remove
    std::string go(int n_times) override
    {
        std::string result;
        for (int i = 0; i < n_times; ++i)
            result += "woof! ";
        return result;
    }
};

inline std::string call_go(Animal *animal)
{
    return animal->go(3);
}

//---------------------------------------------
// TODO: Add more sections...
//---------------------------------------------

} // namespace chimera_test
