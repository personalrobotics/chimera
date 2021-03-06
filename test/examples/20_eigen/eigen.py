import unittest

import eigen_pybind11 as py11
import eigen_boost_python as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):

        self.assertTrue(hasattr(py11, 'print_size'))
        self.assertTrue(hasattr(py11, 'print_block'))
        self.assertTrue(hasattr(py11, 'print_max_coeff'))
        self.assertTrue(hasattr(py11, 'print_inv_cond'))

        py11.test_print_size()
        py11.test_print_block()
        py11.test_print_max_coeff()
        py11.test_print_inv_cond()

        self.assertFalse(hasattr(py11, 'function_with_map_param'))
        self.assertTrue(hasattr(py11, 'test_function_with_map_param'))


    def test_function_bp(self):
        self.assertTrue(hasattr(boost, 'print_size'))
        self.assertTrue(hasattr(boost, 'print_block'))
        self.assertTrue(hasattr(boost, 'print_max_coeff'))
        self.assertTrue(hasattr(boost, 'print_inv_cond'))

        boost.test_print_size()
        boost.test_print_block()
        boost.test_print_max_coeff()
        boost.test_print_inv_cond()

        # Boost.Python < 1.65.1 doesn't work for Eigen::Map parameter
        # Once Boost.Python >= 1.65.1 becomes the minimum required version, change following check to self.assertTrue(...)
        self.assertFalse(hasattr(boost, 'function_with_map_param'))

        self.assertTrue(hasattr(boost, 'test_function_with_map_param'))


if __name__ == '__main__':
    unittest.main()
