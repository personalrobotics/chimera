#pragma once

#include <type_traits>

namespace chimera_test
{

class Base
{
public:
    Base() = default;
    class Option
    {
    public:
        Option() = default;
    };
};

class Derived : public Base
{
public:
    Derived() = default;
    class Option : public Base::Option
    {
    public:
        Option() = default;
    };
};

} // namespace chimera_test
