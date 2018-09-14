#include "class.h"

namespace chimera_test {
namespace nested_namespace {

//==============================================================================
Animal::Animal()
{
  // Do nothing
}

//==============================================================================
Animal::~Animal()
{
  // Do nothing
}

//==============================================================================
std::string Animal::type() const
{
  return "Animal";
}

//==============================================================================
Dog::Dog()
{
  // Do nothing
}

//==============================================================================
std::string Dog::type() const
{
  return "Dog";
}

//==============================================================================
std::string Dog::pure_virtual_type() const
{
  return "Dog";
}

//==============================================================================
std::string Dog::static_type()
{
  return "Dog";
}

//==============================================================================
Husky::Husky()
{
  // Do nothing
}

//==============================================================================
std::string Husky::type() const
{
  return "Husky";
}

//==============================================================================
std::string Husky::pure_virtual_type() const
{
  return "Husky";
}

//==============================================================================
StrongHusky::StrongHusky()
  : Dog(), Strong()
{
  // Do nothing
}

//==============================================================================
std::string StrongHusky::type() const
{
  return "StrongHusky";
}

//==============================================================================
std::string StrongHusky::pure_virtual_type() const
{
  return "StrongHusky";
}

} // namespace nested_namespace
} // namespace chimera_test
