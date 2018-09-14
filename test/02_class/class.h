#pragma once

#include <string>
#include <functional>
#include <memory>

namespace chimera_test {
namespace nested_namespace {

class Animal
{
public:
  Animal();
  virtual ~Animal();
  virtual std::string type() const;
  virtual std::string pure_virtual_type() const = 0;
};

class Dog : public Animal
{
public:
  Dog();
  std::string type() const override;
  std::string pure_virtual_type() const override;

  static std::string static_type();
};

class Strong {};

class Husky : public Dog
{
public:
  Husky();
  std::string type() const override;
  std::string pure_virtual_type() const override;
};

class StrongHusky : public Dog, Strong
{
public:
  StrongHusky();
  std::string type() const override;
  std::string pure_virtual_type() const override;
};

} // namespace nested_namespace
} // namespace chimera_test
