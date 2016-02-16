/**
 * Automatically generated Boost.Python function binding for '{{function.name}}'.
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

void {{function.mangled_name}}()
{
    {{{prebody}}}
    boost::python::def("{{function.name}}",
        static_cast<{{{function.type}}}>(&{{function.qualified_name}}),
            ({{#function.params}}::boost::python::arg("{{name}}"),
             {{/function.params}}));
    {{{postbody}}}
}
{{{postcontent}}}
{{{footer}}}
