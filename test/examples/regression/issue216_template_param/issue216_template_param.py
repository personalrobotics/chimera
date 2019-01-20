import unittest

try:
    import issue216_template_param_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

try:
    import issue216_template_param_boost_python.nested_namespace as boost
    has_boost_python = True
except:
    has_boost_python = False

class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

    def test_function_bp(self):
        if not has_boost_python:
            return

if __name__ == '__main__':
    unittest.main()
