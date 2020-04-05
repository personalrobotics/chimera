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

} // namespace chimera_test
