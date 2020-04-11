import unittest

try:
    import function_pybind11 as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import function_boost_python as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        self.assertTrue(hasattr(py11, 'void_bool'))
        self.assertTrue(hasattr(py11, 'void_int'))
        self.assertTrue(hasattr(py11, 'void_long'))
        self.assertTrue(hasattr(py11, 'void_float'))
        self.assertTrue(hasattr(py11, 'void_double'))
        # self.assertTrue(hasattr(py11, 'void_a'))

        self.assertTrue(hasattr(py11, 'void_cr_bool'))
        self.assertTrue(hasattr(py11, 'void_cr_int'))
        self.assertTrue(hasattr(py11, 'void_cr_long'))
        self.assertTrue(hasattr(py11, 'void_cr_float'))
        self.assertTrue(hasattr(py11, 'void_cr_double'))
        self.assertTrue(hasattr(py11, 'void_cr_a'))

        self.assertEqual(py11.add(), 3)
        self.assertEqual(py11.add(3, 4), 7)
        self.assertEqual(py11.add(i=5, j=6), 11)

        self.assertEqual(py11.inline_add(), 3)
        self.assertEqual(py11.inline_add(3, 4), 7)
        self.assertEqual(py11.inline_add(i=5, j=6), 11)

        py11.void_return()

        dummy = py11.Dummy()
        dummy.val = 5
        self.assertEqual(py11.void_pointer_param(dummy), 5)

        py11.void_param()

        self.assertFalse(hasattr(py11, 'function_with_suppressed_param'))
        self.assertFalse(
            hasattr(py11, 'function_with_suppressed_template_param'))

    def test_function_bp(self):
        self.assertTrue(hasattr(boost, 'void_bool'))
        self.assertTrue(hasattr(boost, 'void_int'))
        self.assertTrue(hasattr(boost, 'void_long'))
        self.assertTrue(hasattr(boost, 'void_float'))
        self.assertTrue(hasattr(boost, 'void_double'))
        # self.assertTrue(hasattr(boost, 'void_a'))

        self.assertTrue(hasattr(boost, 'void_cr_bool'))
        self.assertTrue(hasattr(boost, 'void_cr_int'))
        self.assertTrue(hasattr(boost, 'void_cr_long'))
        self.assertTrue(hasattr(boost, 'void_cr_float'))
        self.assertTrue(hasattr(boost, 'void_cr_double'))
        self.assertTrue(hasattr(boost, 'void_cr_a'))

        self.assertEqual(boost.add(), 3)
        self.assertEqual(boost.add(3, 4), 7)
        self.assertEqual(boost.add(i=5, j=6), 11)

        self.assertEqual(boost.inline_add(), 3)
        self.assertEqual(boost.inline_add(3, 4), 7)
        self.assertEqual(boost.inline_add(i=5, j=6), 11)

        boost.void_return()

        dummy = boost.Dummy()
        dummy.val = 5
        # TODO: void pointer parameter doesn't work with Boost.Python
        # self.assertEqual(boost.void_pointer_param(dummy), 5)

        boost.void_param()

        self.assertFalse(hasattr(boost, 'function_with_suppressed_param'))
        self.assertFalse(
            hasattr(boost, 'function_with_suppressed_template_param'))


if __name__ == '__main__':
    unittest.main()
