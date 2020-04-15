import unittest

import exceptions_pybind11 as py11
import exceptions_boost_python as boost


class TestExceptions(unittest.TestCase):

    def _test_built_in_exception_translation(self, binding):
        if binding is py11:
            self.assertRaises(RuntimeError, binding.throw_std_exception)
            self.assertRaises(MemoryError, binding.throw_bad_alloc)
            self.assertRaises(ValueError, binding.throw_domain_error)
            self.assertRaises(ValueError, binding.throw_invalid_argument)
            self.assertRaises(ValueError, binding.throw_length_error)
            self.assertRaises(IndexError, binding.throw_out_of_range)
            self.assertRaises(ValueError, binding.throw_range_error)
            # binding.throw_overflow_error is generated only for pybind11 >= 2.5.0
            if hasattr(binding, 'throw_overflow_error'):
                self.assertRaises(OverflowError, binding.throw_overflow_error)
        # TODO: Fill in Boost.Python handling of exceptions.

    def test_built_in_exception_translation(self):
        self._test_built_in_exception_translation(py11)
        self._test_built_in_exception_translation(boost)


if __name__ == '__main__':
    unittest.main()
