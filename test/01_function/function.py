import unittest

try:
    import function_pybind11.nested_function as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import function_boost_python.nested_function as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        self.assertEqual(py11.add(), 3)
        self.assertEqual(py11.add(3, 4), 7)
        self.assertEqual(py11.add(i=5, j=6), 11)

        self.assertEqual(py11.inline_add(), 3)
        self.assertEqual(py11.inline_add(3, 4), 7)
        self.assertEqual(py11.inline_add(i=5, j=6), 11)

        py11.void_return()


    def test_function_bp(self):
        self.assertEqual(boost.add(), 3)
        self.assertEqual(boost.add(3, 4), 7)
        self.assertEqual(boost.add(i=5, j=6), 11)

        self.assertEqual(boost.inline_add(), 3)
        self.assertEqual(boost.inline_add(3, 4), 7)
        self.assertEqual(boost.inline_add(i=5, j=6), 11)

        boost.void_return()


if __name__ == '__main__':
    unittest.main()
