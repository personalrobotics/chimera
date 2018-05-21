import function_pybind11
import function_boost_python

print(help(function_pybind11))
print(help(function_boost_python))

print(function_pybind11.nested_function.add(1, 2))
print(function_boost_python.nested_function.add(1, 2))

# Keyword arguments
print(function_pybind11.nested_function.add(j=1, i=2))
print(function_boost_python.nested_function.add(j=1, i=2))

# Default arguments
print(function_pybind11.nested_function.add())
print(function_boost_python.nested_function.add())
