/**
 * Automatically generated Boost.Python chimera module '{{module.name}}'.
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
  void {{.}}();
  {{.}}();

  {{/module.bindings}}
  {{{postbody}}}
}
{{{postcontent}}}
{{{footer}}}