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
    static int add(int a, int b, int c = 0)
    {
        return a + b + c;
    }

private:
    int m_val;
};

} // namespace chimera_test
