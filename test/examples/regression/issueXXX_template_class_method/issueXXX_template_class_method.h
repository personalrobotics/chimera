#pragma once

#include <array>

#define CHIMERA_TEST_STATIC_ASSERT(condition, msg)                             \
    static_assert(condition, #msg);

#define CHIMERA_TESET_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(size)                 \
    CHIMERA_TEST_STATIC_ASSERT(                                                \
        Dim == size, THIS_METHOD_IS_ONLY_FOR_VECTORS_OF_A_SPECIFIC_SIZE)

namespace chimera_test
{

template <typename T, int Dim>
class Vector
{
public:
    Vector() = default;

    Vector(const T &x)
    {
        CHIMERA_TESET_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(1)
        m_data[0] = x;
    }

    Vector(const T &x, const T &y)
    {
        CHIMERA_TESET_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(2)
        m_data[0] = x;
        m_data[1] = y;
    }

private:
    std::array<T, Dim> m_data;
};

inline void foo()
{
    auto vec1d = Vector<double, 1>();
    auto vec2d = Vector<double, 2>();
}

// template class Vector<double, 1>;

// template class Vector<double, 2>;

} // namespace chimera_test
