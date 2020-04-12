import unittest

import classes_pybind11 as py11
import classes_boost_python as boost


class TestClasses(unittest.TestCase):

    def _test_function(self, binding):
        d = binding.Dog()
        self.assertEqual(binding.call_go(d), "woof! woof! woof! ")

        if binding is py11:
            class Cat(binding.Animal):
                def go(self, n_times):
                    return "meow! " * n_times

            c = Cat()
            binding.call_go(c)
            self.assertEqual(binding.call_go(c), "meow! meow! meow! ")

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
