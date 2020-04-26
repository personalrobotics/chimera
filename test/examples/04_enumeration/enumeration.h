#pragma once

namespace chimera_test
{

struct Animal
{
    enum Type
    {
        Dog = 0,
        Cat
    };

    Animal(Type type) : type(type)
    {
    }

    Type type;
};

namespace test1
{

// Anonymous enum
enum
{
    TRUE,
    FALSE,
};

} // namespace test1

} // namespace chimera_test
