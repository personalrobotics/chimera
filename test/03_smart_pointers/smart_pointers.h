#pragma once

#include <string>
#include <functional>
#include <memory>

namespace chimera_test {
namespace nested_namespace {

class Example {};

std::unique_ptr<Example> create_example();

// Taking unique_ptr by value or reference is not allowed by pybind11
// See: https://pybind11.readthedocs.io/en/master/advanced/smart_ptrs.html#std-unique-ptr
// void take_unique_ptr_by_value(std::unique_ptr<Example> example);
// void take_unique_ptr_by_reference(std::unique_ptr<Example>& example);

class ExampleShared {};

// By default, pybind11 holds objects as unique_ptr. In order to use shared_ptr,
// you should specify the hold type in the Chimera configuration file something
// like:
//
//   classes:
//     'ExampleShared':
//       held_type: 'std::shared_ptr<ExampleShared>'
//
// Otherwise, it will causes segfaults.
std::shared_ptr<ExampleShared> create_example_shared();

} // namespace nested_namespace
} // namespace chimera_test
