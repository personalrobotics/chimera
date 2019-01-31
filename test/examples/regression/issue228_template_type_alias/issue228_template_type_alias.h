#pragma once

#include <memory>

namespace chimera_test {
namespace nested_namespace {

template <class T> using custom_shared_ptr = std::shared_ptr<T>;
void take_custom_shared_ptr(custom_shared_ptr<int> val) {}

} // namespace nested_namespace
} // namespace chimera_test
