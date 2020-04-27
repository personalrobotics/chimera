import unittest

import issue262_static_method_boost_python as boost
import issue262_static_method_pybind11 as py11


class TestIssue262(unittest.TestCase):

    def _test_issue262(self, binding):
        # Call static method
        self.assertEqual(binding.Integer.static.add(1, 2), 3)

        # Call instance method
        i = binding.Integer(1)
        self.assertEqual(i.add(2), 3)

    def test_issue262(self):
        self._test_issue262(boost)
        self._test_issue262(py11)


if __name__ == '__main__':
    unittest.main()
