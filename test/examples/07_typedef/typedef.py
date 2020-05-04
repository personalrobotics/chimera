import unittest

import typedef_pybind11 as py11
import typedef_boost_python as boost


class TestTypedef(unittest.TestCase):

    def _test_typedef(self, binding):
        # Not binded because the underlying type, std::string, is not binded
        # TODO: Fix to generate bind since std::string is binded by Boost.Python and pybind11
        self.assertFalse(hasattr(binding.test2, 'String'))
        self.assertFalse(hasattr(binding.test2, 'String2'))

        # Regular class bind
        self.assertTrue(hasattr(binding.test1, 'Position'))

        # Type alias in different scope
        self.assertTrue(hasattr(binding.test2, 'Position'))
        self.assertTrue(hasattr(binding.test2, 'Position2'))

        # Not binded because of missing binding for the template class
        self.assertFalse(hasattr(binding.test2, 'Vectori'))
        self.assertFalse(hasattr(binding.test2, 'Vectord'))

        # Same as Position and Position2 but using typedef
        self.assertTrue(hasattr(binding.test2, 'PositionTypedef'))
        self.assertTrue(hasattr(binding.test2, 'PositionTypedef2'))

        # Same as Vectori and Vectord but using typedef
        self.assertFalse(hasattr(binding.test2, 'VectoriTypedef'))
        self.assertFalse(hasattr(binding.test2, 'VectordTypedef'))

    def test_typedef(self):
        self._test_typedef(boost)
        self._test_typedef(py11)


if __name__ == '__main__':
    unittest.main()
