import unittest
import sys


class TestFunctions(unittest.TestCase):

    def _test_function(self, binding):
        if binding is self.py11:
            self.assertTrue(hasattr(binding, 'print_dict'))
            binding.print_dict({'foo': 123, 'bar': 'hello'})

    def test_function(self):
        self._test_function(self.py11)
        self._test_function(self.boost)


if __name__ == '__main__':
    prefix = sys.argv.pop()

    TestFunctions.py11 = None
    TestFunctions.boost = None

    for arg in sys.argv:
        if arg is 'boost_python':
            TestFunctions.boost = __import__(prefix + '_boost_python')
        elif arg is 'pybind11':
            TestFunctions.py11 = __import__(prefix + '_pybind11')

    unittest.main()
