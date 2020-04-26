#pragma once

#include <vector>

namespace chimera_test
{

template <typename T>
class Vector
{
public:
    Vector() = default;

    void resize(int dim)
    {
        m_data.resize(dim);
    }

    int size() const
    {
        return m_data.size();
    }

private:
    std::vector<T> m_data;
};

// Explicit instantiation declaration
template class Vector<double>;

} // namespace chimera_test
