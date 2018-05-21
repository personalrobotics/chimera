import function_pybind11
import function_boost_python

print(function_pybind11.nested_function.add(1, 2))
print(function_boost_python.nested_function.add(1, 2))
