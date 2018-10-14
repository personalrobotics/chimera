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
    auto constPtrVec = std::vector<const Node*>();
    vec_param(vec);          // ok
    vec_param(ptrVec);       // now ok!
    vec_param(constPtrVec);  // now ok!

    auto arr = std::array<Node, 1>();
    auto ptrArr = std::array<Node*, 1>();
    auto constPtrArr = std::array<const Node*, 1>();
    arr_param(arr);          // ok
    arr_param(ptrArr);       // now ok!
    arr_param(constPtrArr);  // now ok!

    auto set = std::set<Node>();
    auto ptrSet = std::set<Node*>();
    auto constPtrSet = std::set<const Node*>();
    set_param(set);          // ok
    set_param(ptrSet);       // now ok!
    set_param(constPtrSet);  // now ok!
  }
};

} // namespace nested_namespace
} // namespace chimera_test
