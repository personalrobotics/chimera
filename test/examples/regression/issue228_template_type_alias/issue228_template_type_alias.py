import unittest

import issue228_template_type_alias_pybind11.nested_namespace as py11
import issue228_template_type_alias_boost_python.nested_namespace as boost

class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        pass

    def test_function_bp(self):
        pass

if __name__ == '__main__':
    unittest.main()
