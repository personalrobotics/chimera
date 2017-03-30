#ifndef __CHIMERA_BINDING_H__
#define __CHIMERA_BINDING_H__

#include <string>

namespace chimera
{

/**
 * Interface for defining a built-in language binding.
 */
struct Binding {
    std::string class_cpp;
    std::string enum_cpp;
    std::string function_cpp;
    std::string module_cpp;
    std::string variable_cpp;
};

}

#endif // __CHIMERA_BINDING_H__