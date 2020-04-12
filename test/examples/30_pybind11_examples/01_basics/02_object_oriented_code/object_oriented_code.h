#pragma once

#include <memory>
#include <string>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/classes.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

namespace test1
{

//------------------------------------------------------------------
// Creating bindings for a custom type
// & Instance and static fields
// & Dynamic attributes
//------------------------------------------------------------------

struct Pet
{
    Pet(const std::string &name) : name(name)
    {
    }
    void setName(const std::string &name_)
    {
        name = name_;
    }
    const std::string &getName() const
    {
        return name;
    }

    std::string name;
};

//---------------------------------------
// Inheritance and automatic downcasting
//---------------------------------------

struct Dog : Pet
{
    Dog(const std::string &name) : Pet(name)
    {
    }
    std::string bark() const
    {
        return "woof!";
    }
};

inline std::unique_ptr<Pet> create_pet()
{
    return std::unique_ptr<Pet>(new Dog("Molly"));
}

struct PolymorphicPet
{
    virtual ~PolymorphicPet() = default;
};

struct PolymorphicDog : PolymorphicPet
{
    std::string bark() const
    {
        return "woof!";
    }
};

inline std::unique_ptr<PolymorphicPet> create_polymorphic_pet()
{
    return std::unique_ptr<PolymorphicPet>(new PolymorphicDog);
}

} // namespace test1

namespace test2
{

//---------------------------------------
// Overloaded methods
//---------------------------------------

struct Pet
{
    Pet(const std::string &name, int age) : name(name), age(age)
    {
    }

    void set(int age_)
    {
        age = age_;
    }
    void set(const std::string &name_)
    {
        name = name_;
    }

    std::string name;
    int age;
};

} // namespace test2

namespace test3
{

//---------------------------------
// Enumerations and internal types
//---------------------------------

struct Pet
{
    enum Kind
    {
        Dog = 0,
        Cat
    };

    Pet(const std::string &name, Kind type) : name(name), type(type)
    {
    }

    std::string name;
    Kind type;
};

} // namespace test3

} // namespace chimera_test
