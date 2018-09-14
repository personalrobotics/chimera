#pragma once

#include <string>

namespace chimera_test {
namespace nested_namespace {

class Base
{
public:
  Base();
  virtual ~Base();
  virtual std::string virtual_function() const;
  virtual std::string pure_virtual_function() const = 0;
};

class DerivedA : public Base
{
public:
  DerivedA();
  std::string virtual_function() const override;
  std::string pure_virtual_function() const override;
};

class DerivedB : public Base
{
public:
  DerivedB();
  std::string virtual_function() const override;
  std::string pure_virtual_function() const override;
};

} // namespace nested_namespace
} // namespace chimera_test
