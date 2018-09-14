#include "virtual_function.h"

namespace chimera_test {
namespace nested_namespace {

Base::Base() {}

Base::~Base() {}

std::string Base::virtual_function() const { return "Base"; }

DerivedA::DerivedA() {}

std::string DerivedA::virtual_function() const { return "DerivedA"; }

std::string DerivedA::pure_virtual_function() const { return "DerivedA"; }

DerivedB::DerivedB() {}

std::string DerivedB::virtual_function() const { return "DerivedB"; }

std::string DerivedB::pure_virtual_function() const { return "DerivedB"; }


} // namespace nested_namespace
} // namespace chimera_test
