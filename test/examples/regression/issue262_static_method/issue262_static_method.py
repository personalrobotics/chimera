import unittest

import issue262_static_method_boost_python as boost
import issue262_static_method_pybind11 as py11


class TestIssue262(unittest.TestCase):

    def _test_issue262(self, binding):
        # Should be skipped as this static method overloads instance method
        with self.assertRaises(TypeError):
            binding.Integer.add(1, 2)
        self.assertEqual(binding.Integer.add_static(1, 2), 3)

        # Call instance method
        i = binding.Integer(1)
        self.assertEqual(i.add(2), 3)

    def test_issue262(self):
        self._test_issue262(boost)
        self._test_issue262(py11)


if __name__ == '__main__':
    unittest.main()
