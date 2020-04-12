import unittest

import functions_pybind11 as py11
import functions_boost_python as boost


class TestFunctions(unittest.TestCase):

    def _test_function(self, binding):
        if binding is py11:
            self.assertTrue(hasattr(binding, 'print_dict'))
            binding.print_dict({'foo': 123, 'bar': 'hello'})

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
