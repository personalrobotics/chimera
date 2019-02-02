#pragma once

#include <vector>

namespace chimera_test {
namespace nested_namespace {

template <class T>
using my_vector = std::vector<T>;

void take_template_type_alias(const my_vector<int>& vec);

} // namespace nested_namespace
} // namespace chimera_test
