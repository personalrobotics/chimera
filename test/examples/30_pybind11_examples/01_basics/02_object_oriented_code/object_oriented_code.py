import unittest

import object_oriented_code_pybind11 as py11
import object_oriented_code_boost_python as boost


class TestObjectOrientedCode(unittest.TestCase):

    def _test_function(self, binding):
        binding.Pet('Molly')

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
