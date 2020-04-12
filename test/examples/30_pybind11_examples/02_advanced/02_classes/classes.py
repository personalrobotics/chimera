import unittest

import classes_pybind11 as py11
import classes_boost_python as boost


class TestClasses(unittest.TestCase):

    def _test_overriding_virtual_functions_in_python(self, binding):
        d = binding.test1.Dog()
        self.assertEqual(binding.test1.call_go(d), "woof! woof! woof! ")

        if binding is py11:
            class Cat(binding.test1.Animal):
                def go(self, n_times):
                    return "meow! " * n_times

            c = Cat()
            self.assertEqual(binding.test1.call_go(c), "meow! meow! meow! ")

    def test_overriding_virtual_functions_in_python(self):
        self._test_overriding_virtual_functions_in_python(py11)
        self._test_overriding_virtual_functions_in_python(boost)

    def _test_combining_virtual_functions_and_inheritance(self, binding):
        if binding is py11:
            class Poodle(binding.test2.Dog):
                def name(self):
                    return "Poodle"

            p = Poodle()
            self.assertEqual(p.name(), "Poodle")

            class MyHusky(binding.test2.Husky):
                def name(self):
                    return "MyHusky"

            h = MyHusky()
            self.assertEqual(h.name(), "MyHusky")

    def test_combining_virtual_functions_and_inheritance(self):
        self._test_combining_virtual_functions_and_inheritance(py11)
        self._test_combining_virtual_functions_and_inheritance(boost)


if __name__ == '__main__':
    unittest.main()
