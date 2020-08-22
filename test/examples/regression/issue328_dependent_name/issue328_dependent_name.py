import unittest

import issue328_dependent_name_pybind11 as py11
import issue328_dependent_name_boost_python as boost


class TestIssue328(unittest.TestCase):

    def _test_issue328(self, binding):
        # TODO: The next line should be uncommented once #328 is resolved.
        # self.assertEqual(binding.VectorScalar, float)
        pass

    def test_issue328(self):
        self._test_issue328(py11)
        self._test_issue328(boost)


if __name__ == '__main__':
    unittest.main()
