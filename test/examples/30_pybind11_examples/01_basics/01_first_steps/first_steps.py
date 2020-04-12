import unittest

import first_steps_pybind11 as py11
import first_steps_boost_python as boost


class TestFirstSteps(unittest.TestCase):

    def _test_function(self, binding):
        # Test function docstring
        self.assertTrue("Returns sum of two integers" in binding.add.__doc__)

        self.assertEqual(binding.add(1, 2), 3)
        self.assertEqual(binding.add(i=3, j=4), 7)

        self.assertEqual(binding.add_def_args(), 3)
        self.assertEqual(binding.add_def_args(2), 4)
        self.assertEqual(binding.add_def_args(3, 4), 7)
        self.assertEqual(binding.add_def_args(i=5, j=6), 11)

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
