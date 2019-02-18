#pragma once

#include <vector>

namespace chimera_test {

template <class T>
using my_vector = std::vector<T>;

void take_template_type_alias(const my_vector<int>& vec);

} // namespace chimera_test
