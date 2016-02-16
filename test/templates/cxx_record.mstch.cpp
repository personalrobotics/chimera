/**
 * Automatically generated class binding for '{{class.name}}'.
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

void {{class.mangled_name}}
{
    {{{prebody}}}
    ::boost::python::class_<{{{class.qualified_name}}},
        ::boost::python::bases<{{#class.bases}}{{{qualified_name}}}
                               {{/class.bases}}> >("{{class.name}}")

    /* constructors */
    {{#class.constructors}}
    .def(::boost::python::init<{{#params}}{{{type}}}
                               {{/params}}>())
    {{/class.constructors}}

    /* methods */
    {{#class.methods}}
    .def("{{name}}",
         static_cast<{{{type}}}>(&{{qualified_name}}){{#return_value_policy}},
         ::boost::python::return_value_policy<boost::python::copy_const_reference >(){{/return_value_policy}})
    {{/class.methods}}

    /* static methods */
    {{#class.static_methods}}
    .def("{{name}}",
         static_cast<{{{type}}}>(&{{qualified_name}}))
    .staticmethod("{{name}}")
    {{/class.static_methods}}

    /* fields */
    {{#class.fields}}
    {{#is_assignable}}.def_readwrite{{/is_assignable}}{{^is_assignable}}.def_readonly{{/is_assignable}}("{{name}}",
        &{{qualified_name}})
    {{/class.fields}}

    {{{postbody}}};
}
{{{postcontent}}}
{{{footer}}}
