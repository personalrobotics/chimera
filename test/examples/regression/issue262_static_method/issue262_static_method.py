import unittest

import issue262_static_method_boost_python as boost
import issue262_static_method_pybind11 as py11


class TestIssue262(unittest.TestCase):

    def _test_issue262(self, binding):
        # Should be skipped as this static method overloads instance method
        if binding is py11:
            with self.assertRaises(TypeError):
                binding.Integer.add(1, 2)
        elif binding is boost:
            # For some reason, Boost.Python allows instance and static method overloading.
            # However, the limitation is that the overloaded "static method" cannot be
            # called without creating an instance of the class.
            # For the details see: https://github.com/personalrobotics/chimera/issues/262
            i = binding.Integer(1)
            self.assertEqual(i.add(1, 2), 3)

        self.assertEqual(binding.Integer.add_static(1, 2), 3)

        # Call instance method
        i = binding.Integer(1)
        self.assertEqual(i.add(2), 3)

    def test_issue262(self):
        self._test_issue262(boost)
        self._test_issue262(py11)


if __name__ == '__main__':
    unittest.main()
