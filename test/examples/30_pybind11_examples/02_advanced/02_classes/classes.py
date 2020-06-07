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

    def _test_operator_overloading(self, binding):
        v1 = binding.test3.Vector2(1, 2)
        v2 = binding.test3.Vector2(3, -1)

        self.assertEqual(v1.get_x(), 1)
        self.assertEqual(v1.get_y(), 2)
        self.assertEqual(v2.get_x(), 3)
        self.assertEqual(v2.get_y(), -1)

        self.assertAlmostEqual((v1 + v2).get_x(), 4)
        self.assertAlmostEqual((v1 + v2).get_y(), 1)

        self.assertAlmostEqual((v1 - v2).get_x(), -2)
        self.assertAlmostEqual((v1 - v2).get_y(), 3)

        self.assertAlmostEqual((v1 * v2).get_x(), 3)
        self.assertAlmostEqual((v1 * v2).get_y(), -2)

        self.assertAlmostEqual((v1 / v2).get_x(), 1/3)
        self.assertAlmostEqual((v1 / v2).get_y(), -2)

        self.assertAlmostEqual((v1 + 8).get_x(), 9)
        self.assertAlmostEqual((v1 + 8).get_y(), 10)

        self.assertAlmostEqual((v1 - 2).get_x(), -1)
        self.assertAlmostEqual((v1 - 2).get_y(), 0)

        self.assertAlmostEqual((v1 * 8).get_x(), 8)
        self.assertAlmostEqual((v1 * 8).get_y(), 16)

        self.assertAlmostEqual((v1 / 2).get_x(), 0.5)
        self.assertAlmostEqual((v1 / 2).get_y(), 1.0)

        v1 += v2
        self.assertEqual(v1.get_x(), 4)
        self.assertEqual(v1.get_y(), 1)

        v1 -= v2
        self.assertEqual(v1.get_x(), 1)
        self.assertEqual(v1.get_y(), 2)

        v1 *= v2
        self.assertEqual(v1.get_x(), 3)
        self.assertEqual(v1.get_y(), -2)

        v1 /= v2
        self.assertEqual(v1.get_x(), 1)
        self.assertEqual(v1.get_y(), 2)

        v3 = v1.__iadd__(v2)
        self.assertEqual(v1.get_x(), 4)
        self.assertEqual(v1.get_y(), 1)
        self.assertEqual(v3.get_x(), 4)
        self.assertEqual(v3.get_y(), 1)

        # TODO: Non-member operators are not supported
        # v3 = 2 + v1
        # self.assertEqual(v3.get_x(), 3)
        # self.assertEqual(v3.get_y(), 4)

        # TODO: Non-member operators are not supported
        # v3 = 1 - v1
        # self.assertEqual(v3.get_x(), 0)
        # self.assertEqual(v3.get_y(), -1)

        # TODO: Non-member operators are not supported
        # v3 = 3 * v1
        # self.assertEqual(v3.get_x(), 3)
        # self.assertEqual(v3.get_y(), 6)

        # TODO: Non-member operators are not supported
        # v3 = 4 / v1
        # self.assertEqual(v3.get_x(), 1)
        # self.assertEqual(v3.get_y(), 2)

    def test_operator_overloading(self):
        self._test_operator_overloading(py11)
        self._test_operator_overloading(boost)


if __name__ == '__main__':
    unittest.main()
