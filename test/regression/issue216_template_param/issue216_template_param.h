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
    vec_param(vec);       // ok
    vec_param(ptrVec);    // now ok!
    // vec_param(constPtrVec);  // error!
    // GENERATED CODE:
    //
    // ::boost::python::def(
    //     "vec_param",
    //     +[](const std::vector<chimera_test::nested_namespace::const Node *> & _arg0_)
    //         { chimera_test::nested_namespace::vec_param<const chimera_test::nested_namespace::Node *>(_arg0_); },
    //     (::boost::python::arg("_arg0_")));
    //
    // EXPECTED CODE:
    //
    // chimera_test::nested_namespace::const Node *  // this line should be the next code
    // const chimera_test::nested_namespace::Node *  // note the location of 'const'

    auto arr = std::array<Node, 1>();
    auto ptrArr = std::array<Node*, 1>();
    arr_param(arr);     // ok
    arr_param(ptrArr);  // now ok!

    auto set = std::set<Node>();
    auto ptrSet = std::set<Node*>();
    set_param(set);     // ok
    set_param(ptrSet);  // now ok!
  }
};

} // namespace nested_namespace
} // namespace chimera_test
