import unittest

import issue105_extra_commas_boost_python as boost
import issue105_extra_commas_pybind11 as py11

class TestIssue105(unittest.TestCase):

    def _test_issue105(self, binding):
        self.assertTrue(hasattr(binding, 'Base1'))
        self.assertTrue(hasattr(binding, 'Base2'))
        self.assertTrue(hasattr(binding, 'Derived'))

    def test_issue105(self):
        self._test_issue105(boost)
        self._test_issue105(py11)


if __name__ == '__main__':
    unittest.main()
