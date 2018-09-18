import unittest

try:
    import assimp_example_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import assimp_example_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        test = py11.AssimpInputResourceAdaptor()

    def test_function_bp(self):
        test = boost.AssimpInputResourceAdaptor()


if __name__ == '__main__':
    unittest.main()
