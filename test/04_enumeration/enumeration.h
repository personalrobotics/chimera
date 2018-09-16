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

} // namespace nested_namespace
} // namespace chimera_test
