/**
 * Automatically generated Boost.Python class binding for '{{class.name}}'.
 */
{{{header}}}
{{#includes}}
#include <{{{.}}}>
{{/includes}}
{{{precontent}}}

void {{class.mangled_name}}
{
    {{{prebody}}}
    ::boost::python::class_<{{{class.qualified_name}}}{{^class.is_copyable}},
        ::boost::noncopyable{{/class.is_copyable}}{{#class.bases?}},
        ::boost::python::bases<{{#class.bases}}{{{qualified_name}}}{{^last}},
                               {{/last}}{{/class.bases}}>{{/class.bases?}}>("{{class.name}}", boost::python::no_init)

    /* constructors */
    {{#class.constructors}}
    .def(::boost::python::init<{{#params}}{{{type}}}{{^last}},
                               {{/last}}{{/params}}>
            ({{#params?}}({{#params}}::boost::python::arg("{{name}}"){{^last}},
              {{/last}}{{/params}}){{/params?}}))
    {{/class.constructors}}

    /* methods */
    {{#class.methods}}
    .def("{{name}}",
        static_cast<{{{type}}}>(&{{qualified_name}}){{#params?}},
            ({{#params}}::boost::python::arg("{{name}}"){{^last}},
             {{/last}}{{/params}}){{/params?}}{{#return_value_policy}},
            ::boost::python::return_value_policy<{{{.}}}>(){{/return_value_policy}}){{#is_static}}
    .staticmethod("{{name}}"){{/is_static}}
    {{/class.methods}}

    /* fields */
    {{#class.fields}}
    {{#is_assignable}}.def_readwrite{{/is_assignable}}{{^is_assignable}}.def_readonly{{/is_assignable}}("{{name}}",
        &{{qualified_name}})
    {{/class.fields}}

    {{{postbody}}};
}
{{{postcontent}}}
{{{footer}}}
