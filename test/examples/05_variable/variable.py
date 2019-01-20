import unittest

try:
    import variable_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import variable_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        self.assertEqual(py11.const_int_val, 10)

    def test_function_bp(self):
        self.assertEqual(boost.const_int_val, 10)


if __name__ == '__main__':
    unittest.main()
