import unittest

# import issueXXX_template_class_method_pybind11 as py11
import issueXXX_template_class_method_boost_python as boost


class TestIssueXXX(unittest.TestCase):

    def _test_issueXXX(self, binding):
        self.assertTrue(hasattr(binding, 'Vector1d'))
        self.assertTrue(hasattr(binding, 'Vector2d'))

    def test_issueXXX(self):
        # self._test_issueXXX(py11)
        self._test_issueXXX(boost)

if __name__ == '__main__':
    unittest.main()
