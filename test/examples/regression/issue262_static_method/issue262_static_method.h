#pragma once

#include <type_traits>

namespace chimera_test
{

class Integer
{
public:
    Integer(int val) : m_val(val)
    {
    }
    int add(int a)
    {
        return a + m_val;
    }
    // Should be skipped as this static method overloads instance method
    static int add(int a, int b)
    {
        return a + b;
    }
    static int add_static(int a, int b)
    {
        return a + b;
    }

private:
    int m_val;
};

} // namespace chimera_test
