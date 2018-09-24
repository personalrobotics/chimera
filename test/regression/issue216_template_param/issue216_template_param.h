#pragma once

#include <array>
#include <set>
#include <vector>
#include <boost/python.hpp>

namespace chimera_test {
namespace nested_namespace {

template <typename T>
void val_param(T /*val*/) { }

template <typename T>
void vec_param(const std::vector<T>& /*vec*/) { }

template <typename T>
void arr_param(const std::array<T, 1>& /*arr*/) { }

template <typename T>
void set_param(const std::set<T>& /*set*/) { }

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
    vec_param(vec);       // ok
    vec_param(ptrVec);  // error!
    // error: ‘Node’ was not declared in this scope
    // sm.def(
    //     "vector_param",
    //     +[](const std::vector<Node *> & vec) -> void
    //         { return chimera_test::common::vector_param<chimera_test::common::Node *>(vec); },
    //     ::pybind11::arg("vec"));

    auto arr = std::array<Node, 1>();
    auto ptrArr = std::array<Node*, 1>();
    arr_param(arr);     // ok
    // arr_param(ptrArr);  // same error!

    auto set = std::set<Node>();
    auto ptrSet = std::set<Node*>();
    set_param(set);     // ok
    // set_param(ptrSet);  // same error!
  }
};

} // namespace nested_namespace
} // namespace chimera_test
