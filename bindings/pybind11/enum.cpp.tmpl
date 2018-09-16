{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{#sources}}
#include <{{.}}>
{{/sources}}
{{{precontent}}}
#include <pybind11/pybind11.h>
{{postinclude}}

void {{enum.mangled_name}}(pybind11::module& m)
{
    auto sm = m{{!
        }}{{#enum.scope}}{{#name}}.def_submodule("{{name}}"){{/name}}{{/enum.scope}};

    ::pybind11::enum_<{{{enum.type}}}>(sm, "{{enum.name}}"){{#enum.values}}
        .value("{{name}}", {{{qualified_name}}}){{/enum.values}}
        .export_values();
}
{{{postcontent}}}
{{{footer}}}
