import unittest

try:
    from function_pybind11.nested_function import add as add_py11
    from function_pybind11.nested_function import inline_add as inline_add_py11
    has_pybind11 = True
except:
    has_pybind11 = False

from function_boost_python.nested_function import add as add_bp
from function_boost_python.nested_function import inline_add as inline_add_bp


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        self.assertEqual(add_py11(), 3)
        self.assertEqual(add_py11(3, 4), 7)
        self.assertEqual(add_py11(i=5, j=6), 11)

        self.assertEqual(inline_add_py11(), 3)
        self.assertEqual(inline_add_py11(3, 4), 7)
        self.assertEqual(inline_add_py11(i=5, j=6), 11)

    def test_function_bp(self):
        self.assertEqual(add_bp(), 3)
        self.assertEqual(add_bp(3, 4), 7)
        self.assertEqual(add_bp(i=5, j=6), 11)

        self.assertEqual(inline_add_bp(), 3)
        self.assertEqual(inline_add_bp(3, 4), 7)
        self.assertEqual(inline_add_bp(i=5, j=6), 11)


if __name__ == '__main__':
    unittest.main()
