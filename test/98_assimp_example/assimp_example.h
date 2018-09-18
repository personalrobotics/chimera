#pragma once

#include <string>
#include <assimp/IOStream.hpp>

namespace chimera_test {
namespace nested_namespace {

class AssimpInputResourceAdaptor : public Assimp::IOStream
{
public:
  AssimpInputResourceAdaptor() = default;
};

} // namespace nested_namespace
} // namespace chimera_test
