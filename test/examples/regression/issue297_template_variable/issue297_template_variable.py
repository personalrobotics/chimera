import unittest

import issue297_template_variable_pybind11 as py11
import issue297_template_variable_boost_python as boost


class TestIssue297(unittest.TestCase):

    def _test_issue297(self, binding):
        self.assertFalse(hasattr(binding, 'template_var'))
        self.assertTrue(hasattr(binding, 'specialized_var'))

    def test_issue297(self):
        self._test_issue297(py11)
        self._test_issue297(boost)

if __name__ == '__main__':
    unittest.main()
