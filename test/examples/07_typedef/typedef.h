#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace chimera_test
{

namespace test1
{

class Position
{
public:
    Position() = default;
};

template <typename T>
class Vector
{
public:
    Vector() = default;
};

} // namespace test1

namespace test2
{

// Not binded because the underlying type, std::string, is not binded
// TODO: Fix to generate bind since std::string is binded by Boost.Python and
// pybind11
using String = std::string;
using String2 = String;

using Position = test1::Position;
using Position2 = Position;

// Not binded because it's a templated type alias
template <typename T>
using Vector = test1::Vector<T>;

// Not binded because of missing binding for the template class
using Vectori = test1::Vector<int>;
using Vectord = Vector<double>;

// Not binded because the underlying type, std::string, is not binded
// TODO: Fix to generate bind since std::string is binded by Boost.Python and
// pybind11
typedef std::string StringTypedef;
typedef StringTypedef StringTypedef2;

typedef test1::Position PositionTypedef;
typedef PositionTypedef PositionTypedef2;

// Not binded because of missing binding for the template class
typedef test1::Vector<int> VectoriTypedef;
typedef Vector<double> VectordTypedef;

} // namespace test2

} // namespace chimera_test
