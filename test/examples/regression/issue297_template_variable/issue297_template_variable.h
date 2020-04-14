#pragma once

#include <type_traits>

namespace chimera_test
{

template <typename T>
constexpr bool template_var = std::true_type::value;

} // namespace chimera_test
