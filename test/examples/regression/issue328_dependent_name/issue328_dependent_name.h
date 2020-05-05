#pragma once

#include <type_traits>

namespace chimera_test
{

template <typename T>
class Vector
{
public:
    using Scalar = T;

    Vector() = default;
};

template<typename S>
struct ScalarTrait {
  typedef typename S::Scalar type;
};

using VectorScalar = ScalarTrait<Vector<double>>::type;

} // namespace chimera_test
