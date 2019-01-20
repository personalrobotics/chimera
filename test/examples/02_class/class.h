#pragma once

#include <string>
#include <functional>
#include <memory>

namespace chimera_test {
namespace nested_namespace {

//class Animal
//{
//public:
//  Animal();
//  virtual ~Animal();
//  virtual std::string type() const;
//  virtual std::string pure_virtual_type() const = 0;
//};

//class Dog : public Animal
//{
//public:
//  Dog();
//  std::string type() const override;
//  std::string pure_virtual_type() const override;

//  static std::string static_type();
//};

//class Strong {};

//class Husky : public Dog
//{
//public:
//  Husky();
//  std::string type() const override;
//  std::string pure_virtual_type() const override;
//};

//class StrongHusky : public Dog, Strong
//{
//public:
//  StrongHusky();
//  std::string type() const override;
//  std::string pure_virtual_type() const override;
//};

//class DefaultArguments
//{
//public:
//  DefaultArguments() = default;
//  int add(int i = 1, int j = 2) const;
//};

//class StaticFields
//{
//public:
//  static std::string m_static_readwrite_type;
//  static const std::string m_static_readonly_type;
//  static std::string static_type();
//};

class NonPublicParamInConstructor
{
protected:
    using ProtectedTypeDef = int;
  struct ProtectedData{};
  enum ProtectedEnum { PE_ONE };

public:
  NonPublicParamInConstructor(const ProtectedData&) {}
  NonPublicParamInConstructor(ProtectedEnum) {}
  NonPublicParamInConstructor(const std::string& name) : m_name(name) {}
  std::string m_name;
};

namespace detail {

//class ClassInDetail {};

} // namespace detail

} // namespace nested_namespace
} // namespace chimera_test
