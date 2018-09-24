#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace chimera_test {
namespace nested_namespace {

inline std::vector<int> cast_vector() { return std::vector<int>{1}; }

} // namespace nested_namespace
} // namespace chimera_test
