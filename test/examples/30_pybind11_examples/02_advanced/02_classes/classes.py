import unittest

import classes_pybind11 as py11
import classes_boost_python as boost


class TestClasses(unittest.TestCase):

    def _test_override_virtual_functions(self, binding):
        d = binding.test1.Dog()
        self.assertEqual(binding.test1.call_go(d), "woof! woof! woof! ")

        if binding is py11:
            class Cat(binding.test1.Animal):
                def go(self, n_times):
                    return "meow! " * n_times

            c = Cat()
            self.assertEqual(binding.test1.call_go(c), "meow! meow! meow! ")

    def test_override_virtual_functions(self):
        self._test_override_virtual_functions(py11)
        self._test_override_virtual_functions(boost)

    def _test_combining_virtual_functions_and_inheritance(self, binding):
        if binding is py11:
            class ShihTzu(binding.test2.Dog):
                def bark(self):
                    return "yip!"
            s = ShihTzu()
            self.assertEqual(binding.test2.call_bark(s), "yip!")

    def test_combining_virtual_functions_and_inheritance(self):
        self._test_combining_virtual_functions_and_inheritance(py11)
        self._test_combining_virtual_functions_and_inheritance(boost)


if __name__ == '__main__':
    unittest.main()
