#include "functions.h"

namespace chimera_test
{

//==============================================================================
void print_dict(const pybind11::dict &dict)
{
    /* Easily interact with Python types */
    for (auto item : dict)
        std::cout << "key=" << std::string(pybind11::str(item.first)) << ", "
                  << "value=" << std::string(pybind11::str(item.second))
                  << std::endl;
}

} // namespace chimera_test
