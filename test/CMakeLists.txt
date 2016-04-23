include(CTest)
include(chimeraFunctions)

enable_testing()

add_chimera_binding(
  EXCLUDE_FROM_ALL
  TARGET bindingTest0
  SOURCES dart-example.cpp
  NAMESPACES test
)
add_test(ChimeraBindingTest bindingTest0)
