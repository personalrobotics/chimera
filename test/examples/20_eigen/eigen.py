import unittest

# import eigen_pybind11.nested_namespace as py11
import eigen_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    # def test_function_py11(self):
    #     py11.test_print_size()
    #     py11.test_print_block()
    #     py11.test_print_max_coeff()
    #     py11.test_print_inv_cond()


    def test_function_bp(self):
        self.assertTrue(hasattr(boost, 'print_size'))
        self.assertTrue(hasattr(boost, 'print_block'))
        self.assertTrue(hasattr(boost, 'print_max_coeff'))
        self.assertTrue(hasattr(boost, 'print_inv_cond'))

        boost.test_print_size()
        boost.test_print_block()
        boost.test_print_max_coeff()
        boost.test_print_inv_cond()

        self.assertTrue(hasattr(boost, 'function_with_map_param'))
        self.assertTrue(hasattr(boost, 'test_function_with_map_param'))


if __name__ == '__main__':
    unittest.main()
