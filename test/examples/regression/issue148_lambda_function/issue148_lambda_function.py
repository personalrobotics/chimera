import unittest

import issue148_lambda_function_pybind11 as py11
import issue148_lambda_function_boost_python as boost


class TestFunction(unittest.TestCase):

    def _test_lambda_function(self, binding):
        pass

    def test_lambda_function(self):
        self._test_lambda_function(py11)
        self._test_lambda_function(boost)


if __name__ == '__main__':
    unittest.main()
