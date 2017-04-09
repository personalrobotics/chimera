#ifndef __CHIMERA_BINDING_H__
#define __CHIMERA_BINDING_H__

#include <string>
#include <map>

namespace chimera
{
namespace binding
{

/**
 * Interface for defining a built-in language binding.
 */
struct Definition {
    std::string class_cpp;
    std::string enum_cpp;
    std::string function_cpp;
    std::string module_cpp;
    std::string variable_cpp;
};

/**
 * Static map of available built-in binding definitions.
 */
extern std::map<std::string, Definition> DEFINITIONS;

/**
 * Name of default built-in binding.
 *
 * This is defined as a static instead of const so that it can be more easily
 * auto-generated at build time.
 */
extern std::string DEFAULT_NAME;

/**
 * Loads pre-built bindings into Configuration.
 * The body of this function is auto-generated by CMake.
 */
void initializeBuiltinBindings();

} // namespace binding
} // namespace chimera

#endif // __CHIMERA_BINDING_H__
