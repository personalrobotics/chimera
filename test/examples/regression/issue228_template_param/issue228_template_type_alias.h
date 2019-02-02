#pragma once

// #include <memory>

namespace chimera_test {
namespace nested_namespace {

template <typename T>
class MyClass {};

using custom_shared_ptr_float = MyClass<float>;
void take_custom_shared_ptr(custom_shared_ptr_float val) {}

// template <class T> using custom_shared_ptr = std::shared_ptr<T>;
// template <class T> using custom_shared_ptr = MyClass<T>;
// void take_custom_shared_ptr(custom_shared_ptr<int> val) {}

} // namespace nested_namespace
} // namespace chimera_test
