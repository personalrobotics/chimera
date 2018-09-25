#pragma once

namespace chimera_test {
namespace nested_namespace {

struct Animal
{
  enum Type
  {
    Dog = 0,
    Cat
  };

  Animal(Type type) : type(type) { }

  Type type;
};

enum GlobalEnum
{
  GE_TYPE1 = 0,
  GE_TYPE2
};

enum class GlobalEnumClass
{
  TYPE1 = 0,
  TYPE2
};

struct MainStruct
{
  enum Type
  {
    Dog = 0,
    Cat
  };

  enum EnumInStruct
  {
    ICE_TYPE1 = 0,
    ICE_TYPE2
  };

  enum EnumClassInStruct
  {
    TYPE1 = 0,
    TYPE2
  };

  struct NestedStruct
  {
    enum EnumInStruct
    {
      ICE_TYPE1 = 0,
      ICE_TYPE2
    };

    enum EnumClassInStruct
    {
      TYPE1 = 0,
      TYPE2
    };

    NestedStruct() = default;
  };

  MainStruct() = default;
};

} // namespace nested_namespace
} // namespace chimera_test
