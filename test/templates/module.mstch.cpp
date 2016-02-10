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

/** AWW HELL YEAH */
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