/**
 * Automatically generated Boost.Python enum binding for '{{enum.name}}'.
 */
{{header}}
{{#includes}}
#include <{{.}}>
{{/includes}}
{{precontent}}

void {{enum.mangled_name}}()
{
    {{prebody}}
    ::boost::python::enum_<{{enum.qualified_name}}>("{{enum.name}}"){{#enum.values}}
        .value("{{name}}", {{qualified_name}}){{/enum.values}}
    {{postbody}};
}
{{postcontent}}
{{footer}}
