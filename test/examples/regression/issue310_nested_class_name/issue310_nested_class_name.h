#pragma once

#include <type_traits>

namespace chimera_test
{

namespace common
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

class CommonDerivedA : public Base
{
public:
    CommonDerivedA() = default;
    class Option : public Base::Option
    {
    public:
        Option() = default;
    };
};

// class CommonDerivedB : public Base
// {
// public:
//     CommonDerivedB() = default;
//     class NestedClass
//     {
//     public:
//         class Option : public Base::Option
//         {
//         public:
//             Option() = default;
//         };
//     };
// };

} // namespace common

namespace math
{

// class MathDerivedA : public common::Base
// {
// public:
//     MathDerivedA() = default;
//     class Option : public Base::Option
//     {
//     public:
//         Option() = default;
//     };
// };

// class MathDerivedB : public common::Base
// {
// public:
//     MathDerivedB() = default;
//     class NestedClass
//     {
//     public:
//         class Option : public Base::Option
//         {
//         public:
//             Option() = default;
//         };
//     };
// };

} // namespace math

} // namespace chimera_test
