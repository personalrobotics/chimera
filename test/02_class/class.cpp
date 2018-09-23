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

//==============================================================================
int DefaultArguments::add(int i, int j) const
{
  return i + j;
}

//==============================================================================
std::string StaticFields::m_static_readwrite_type = "static readwrite type";
const std::string StaticFields::m_static_readonly_type = "static readonly type";

//==============================================================================
std::string StaticFields::static_type()
{
  static std::string type = "static type";
  return type;
}

} // namespace nested_namespace
} // namespace chimera_test
