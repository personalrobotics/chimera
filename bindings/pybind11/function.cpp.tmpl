{{header}}
{{#includes}}
#include <{{.}}>
{{/includes}}
{{#sources}}
#include <{{.}}>
{{/sources}}
{{precontent}}
#include <pybind11/pybind11.h>
{{postinclude}}

void {{function.mangled_name}}(pybind11::module& m)
{
    auto sm = m{{!
        }}{{#function.scope}}{{#name}}.def_submodule("{{name}}"){{/name}}{{/function.scope}};

{{#function.overloads}}{{!
    }}    sm.def("{{name}}", +[]({{#params}}{{type}} {{name}}{{^last}}, {{/last}}{{/params}}) -> {{return_type}} { {{!
    }}return {{qualified_call}}({{#params}}{{name}}{{^last}}, {{/last}}{{/params}}); }{{!
    }}{{#return_value_policy}}, ::pybind11::return_value_policy<{{.}} >(){{/return_value_policy}}{{!
    }}{{#params?}}, {{#params}}::pybind11::arg("{{name}}"){{^last}}, {{/last}}{{/params}}{{/params?}});
{{/function.overloads}}
}

{{postcontent}}
{{footer}}
