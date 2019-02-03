import unittest

try:
    import issue228_template_type_alias_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False
import issue228_template_type_alias_boost_python.nested_namespace as boost

class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return
        self.assertTrue(hasattr(py11, 'take_template_type_alias'))

    def test_function_bp(self):
        self.assertTrue(hasattr(boost, 'take_template_type_alias'))

if __name__ == '__main__':
    unittest.main()
