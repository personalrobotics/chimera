#pragma once

#include <type_traits>

namespace chimera_test
{

class Base1
{
public:
    Base1() = default;
};

class Base2
{
public:
    Base2() = default;
};

class Derived : public Base1, public Base2
{
public:
    Derived() = default;
};

} // namespace chimera_test
