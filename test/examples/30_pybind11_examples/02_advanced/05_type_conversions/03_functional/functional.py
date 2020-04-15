import unittest

import functional_pybind11 as py11
# import functional_boost_python as boost  # TODO: Fix binding for Boost.Python


class TestFunctional(unittest.TestCase):

    def _test_callbacks_and_passing_anonymous_functions(self, binding):
        def square(i):
            return i * i

        self.assertEqual(binding.func_arg(square), 100)

        square_plus_1 = binding.func_ret(square)
        self.assertEqual(square_plus_1(4), 17)

        plus_1 = binding.func_cpp()
        self.assertEqual(plus_1(number=43), 44)

    def test_callbacks_and_passing_anonymous_functions(self):
        self._test_callbacks_and_passing_anonymous_functions(py11)
        # TODO: Fix binding for Boost.Python
        # self._test_callbacks_and_passing_anonymous_functions(boost)


if __name__ == '__main__':
    unittest.main()
