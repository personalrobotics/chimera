/**
 * Automatically generated Boost.Python global variable binding for '{{variable.name}}'.
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

void {{variable.mangled_name}}
{
    {{{prebody}}}
    ::boost::python::scope().attr("{{variable.name}}") = {{variable.qualified_name}};
    {{{postbody}}}
}
{{{postcontent}}}
{{{footer}}}
