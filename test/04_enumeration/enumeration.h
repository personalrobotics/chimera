#pragma once

namespace chimera_test {
namespace nested_namespace {

enum Type
{
  Dog = 0,
  Cat
};

struct Animal
{
  Animal(Type type) : type(type) { }

  Type type;
};

} // namespace nested_namespace
} // namespace chimera_test
