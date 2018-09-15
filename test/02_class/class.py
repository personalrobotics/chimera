import unittest

try:
    import class_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import class_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        dog = py11.Dog()
        husky = py11.Husky()
        strong_husky = py11.StrongHusky()

        self.assertEqual(dog.type(), 'Dog')
        self.assertEqual(dog.pure_virtual_type(), 'Dog')
        self.assertEqual(py11.Dog.static_type(), 'Dog')

        self.assertEqual(husky.type(), 'Husky')
        self.assertEqual(husky.pure_virtual_type(), 'Husky')

        self.assertEqual(strong_husky.type(), 'StrongHusky')
        self.assertEqual(strong_husky.pure_virtual_type(), 'StrongHusky')

        default_args = py11.DefaultArguments()

        self.assertEqual(default_args.add(), 3)
        self.assertEqual(default_args.add(2), 4)
        self.assertEqual(default_args.add(2, 3), 5)

    def test_function_bp(self):
        dog = boost.Dog()
        husky = boost.Husky()
        strong_husky = boost.StrongHusky()

        self.assertEqual(dog.type(), 'Dog')
        self.assertEqual(dog.pure_virtual_type(), 'Dog')
        self.assertEqual(boost.Dog.static_type(), 'Dog')

        self.assertEqual(husky.type(), 'Husky')
        self.assertEqual(husky.pure_virtual_type(), 'Husky')

        self.assertEqual(strong_husky.type(), 'StrongHusky')
        self.assertEqual(strong_husky.pure_virtual_type(), 'StrongHusky')

        default_args = boost.DefaultArguments()

        self.assertEqual(default_args.add(), 3)
        self.assertEqual(default_args.add(2), 4)
        self.assertEqual(default_args.add(2, 3), 5)


if __name__ == '__main__':
    unittest.main()
