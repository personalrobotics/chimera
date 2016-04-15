include(CTest)
include(chimeraFunctions)

enable_testing()

add_chimera_binding(
  TARGET bindingTest0
  SOURCES binding0.hpp
  NAMESPACES test
)
add_test(ChimeraBindingTest bindingTest0)
