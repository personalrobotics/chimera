#pragma once

#include <functional>

namespace chimera_test
{

inline std::function<void()> foo()
{
    return [] {};
}

} // namespace chimera_test
