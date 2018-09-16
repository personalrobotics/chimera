import unittest

try:
    import smart_pointers_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

try:
    import smart_pointers_boost_python.nested_namespace as boost
    has_boost_python = True
except:
    has_boost_python = False

class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        example_unique_ptr = py11.create_example()
        example_shared_ptr = py11.create_example_shared()

    def test_function_bp(self):
        if not has_boost_python:
            return

        # TODO(JS): Boost.Python requires to specify held_type for smart pointers
        # Following test is disabled for now
        # example_unique_ptr = boost.create_example()

if __name__ == '__main__':
    unittest.main()
