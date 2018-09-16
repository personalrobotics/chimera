import unittest

try:
    import enumeration_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import enumeration_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        animal = py11.Animal(py11.Animal.Cat)
        self.assertEqual(animal.type, py11.Animal.Cat)


    def test_function_bp(self):
        animal = boost.Animal(boost.Animal.Type.Cat)
        self.assertEqual(animal.type, boost.Animal.Type.Cat)


if __name__ == '__main__':
    unittest.main()
