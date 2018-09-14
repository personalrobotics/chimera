import unittest

try:
    import virtual_function_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import virtual_function_boost_python.nested_namespace as bp


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        derivedA = py11.DerivedA()
        derivedB = py11.DerivedB()

        self.assertEqual(derivedA.virtual_function(), 'DerivedA')
        self.assertEqual(derivedA.pure_virtual_function(), 'DerivedA')

        self.assertEqual(derivedB.virtual_function(), 'DerivedB')
        self.assertEqual(derivedB.pure_virtual_function(), 'DerivedB')

    def test_function_bp(self):
        derivedA = bp.DerivedA()
        derivedB = bp.DerivedB()

        self.assertEqual(derivedA.virtual_function(), 'DerivedA')

        self.assertEqual(derivedA.virtual_function(), 'DerivedA')
        self.assertEqual(derivedA.pure_virtual_function(), 'DerivedA')

        self.assertEqual(derivedB.virtual_function(), 'DerivedB')
        self.assertEqual(derivedB.pure_virtual_function(), 'DerivedB')


if __name__ == '__main__':
    unittest.main()
