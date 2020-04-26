import unittest

import template_class_pybind11 as py11
import template_class_boost_python as boost


class TestTemplateClass(unittest.TestCase):

    def _test_template_class(self, binding):
        v = binding.VectorXd()
        self.assertEqual(v.size(), 0)

        v.resize(10)
        self.assertEqual(v.size(), 10)

    def test_template_class(self):
        self._test_template_class(py11)
        self._test_template_class(boost)


if __name__ == '__main__':
    unittest.main()
