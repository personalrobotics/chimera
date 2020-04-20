import unittest

import issue310_nested_class_name_boost_python as boost


class TestIssue310(unittest.TestCase):

    def _test_issue310(self, binding):
        self.assertTrue(hasattr(binding.Base, 'Option'))
        self.assertTrue(hasattr(binding.Derived, 'Option'))

    def test_issue310(self):
        self.assertRaises(ImportError, __import__('issue310_nested_class_name_pybind11'))
        self._test_issue310(boost)


if __name__ == '__main__':
    unittest.main()
