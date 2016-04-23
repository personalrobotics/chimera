include(CTest)
include(chimeraFunctions)

enable_testing()

add_chimera_binding(
  DEBUG
  TARGET bindingTest0
  SOURCES dart-example.cpp
  NAMESPACES test
)
add_test(ChimeraBindingTest bindingTest0)
