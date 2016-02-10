// TODO: this is probably not the right place to put this.

/**
 * Chimera templates to generate boost::python bindings.
 */
std::string CLASS_BINDING_CPP = R"(
/**
 * Automatically generated binding for {{class.name}}.
 * Generated on {{date}}.
 */
)";

/**
 * Chimera templates to generate boost::python bindings.
 */
std::string ENUM_BINDING_CPP = R"(
/**
 * Automatically generated binding for {{enum.name}}.
 * Generated on {{date}}.
 */
)";

std::string VAR_BINDING_CPP = R"(
/**
 * Automatically generated binding for {{variable.name}}.
 * Generated on {{date}}.
 */
)";

std::string FUNCTION_BINDING_CPP = R"(
/**
 * Automatically generated binding for {{function.name}}.
 * Generated on {{date}}.
 */
)";

std::string MODULE_CPP = R"(
/**
 * Automatically generated chimera binding.
 * Generated on {{date}}.
 *
 *
 * To use this binding, compile all included source files, then
 * add the resulting `.so` to your python path and run:
 *
 * >>> import {{module.name}}
 *
 */
)";