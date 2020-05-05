import unittest

import issue328_nested_class_name_pybind11 as py11
import issue328_nested_class_name_boost_python as boost


class TestIssue328(unittest.TestCase):

    def _test_issue328(self, binding):
        pass

    def test_issue328(self):
        self._test_issue328(py11)
        self._test_issue328(boost)


if __name__ == '__main__':
    unittest.main()
