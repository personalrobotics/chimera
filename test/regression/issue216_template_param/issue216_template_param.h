#pragma once

#include <array>
#include <vector>

namespace chimera_test {
namespace nested_namespace {

template <typename T>
inline void val_param(T /*val*/) { }

template <typename T>
inline void vec_param(const std::vector<T>& /*vec*/) { }

template <typename T>
inline void arr_param(const std::array<T, 1>& /*arr*/) { }

class Node
{
public:
  Node() = default;
};

class Skeleton
{
public:
  Skeleton() = default;

  inline void foo()
  {
    val_param(Node());      // ok
    val_param(new Node());  // ok

    auto vec = std::vector<Node>();
    auto ptrVec = std::vector<Node*>();
    vec_param(vec);  // ok
    // error: ‘Node’ was not declared in this scope
    // sm.def(
    //     "vector_param",
    //     +[](const std::vector<Node *> & vec) -> void
    //         { return chimera_test::common::vector_param<chimera_test::common::Node *>(vec); },
    //     ::pybind11::arg("vec"));
    //vec_param(ptrVec);

    auto arr = std::array<Node, 1>();
    auto ptrArr = std::array<Node*, 1>();
    arr_param(arr);
    //arr_param(ptrArr);
  }
};

} // namespace nested_namespace
} // namespace chimera_test
