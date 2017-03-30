// TODO: this is probably not the right place to put this.
#include <chimera/boost_python_mstch.h>

const std::string CLASS_BINDING_CPP = R"(
/**
 * Automatically generated class binding for '{{class.name}}'.
 * Generated on {{date}}.
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

class {{class.mangled_name}}
{
    {{{prebody}}}
    /* constructors */
    /* methods */
    /* static methods */
    /* fields */
    {{{postbody}}}
}
{{{postcontent}}}
{{{footer}}}
)";

const std::string ENUM_BINDING_CPP = R"(
/**
 * Automatically generated enum binding for '{{enum.name}}'.
 * Generated on {{date}}.
 */
)";

const std::string FUNCTION_BINDING_CPP = R"(
/**
 * Automatically generated function binding for '{{function.name}}'.
 * Generated on {{date}}.
 */
)";

const std::string VAR_BINDING_CPP = R"(
/**
 * Automatically generated global variable binding for '{{variable.name}}'.
 * Generated on {{date}}.
 */
)";

const std::string MODULE_CPP = R"(
/**
 * Automatically generated chimera module '{{module.name}}'.
 * Generated on {{date}}.
 *
 * To use this binding, compile all included source files, then
 * add the resulting `.so` to your python path and run:
 *
 * >>> import {{module.name}}
 *
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

BOOST_PYTHON({{module.name}})
{
  {{{prebody}}}
  {{#module.bindings}}
  {{.}}();
  {{/module.bindings}}
  {{{postbody}}}
}
{{{postcontent}}}
{{{footer}}}

)";

const chimera::Binding PYTHON_BINDING {
  CLASS_BINDING_CPP,
  ENUM_BINDING_CPP,
  FUNCTION_BINDING_CPP,
  MODULE_CPP,
  VAR_BINDING_CPP,
};
