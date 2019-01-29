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
        boost.test_print_size()
        boost.test_print_block()
        boost.test_print_max_coeff()
        boost.test_print_inv_cond()


if __name__ == '__main__':
    unittest.main()
