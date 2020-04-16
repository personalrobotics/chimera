import inspect
import unittest

try:
    import class_pybind11 as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import class_boost_python as boost


class TestClass(unittest.TestCase):

    def _test_class(self, binding):
        dog = binding.Dog()
        self.assertEqual(dog.type(), 'Dog')
        self.assertEqual(dog.pure_virtual_type(), 'Dog')
        self.assertEqual(binding.Dog.static_type(), 'Dog')

        husky = binding.Husky()
        self.assertEqual(husky.type(), 'Husky')
        self.assertEqual(husky.pure_virtual_type(), 'Husky')

        strong_husky = binding.StrongHusky()
        self.assertEqual(strong_husky.type(), 'StrongHusky')
        self.assertEqual(strong_husky.pure_virtual_type(), 'StrongHusky')

        default_args = binding.DefaultArguments()
        self.assertEqual(default_args.add(), 3)
        self.assertEqual(default_args.add(2), 4)
        self.assertEqual(default_args.add(2, 3), 5)

        self.assertEqual(binding.StaticFields.m_static_readonly_type, 'static readonly type')
        self.assertEqual(binding.StaticFields.m_static_readwrite_type, 'static readwrite type')
        # TODO(JS): Boost.Python doesn't work for readwrite static field (see: #194)
        if has_pybind11 and binding is py11:
            binding.StaticFields.m_static_readwrite_type = 'new type'
            self.assertEqual(binding.StaticFields.m_static_readwrite_type, 'new type')
        self.assertEqual(binding.StaticFields.static_type(), 'static type')

        non_pub_param_in_ctr = binding.NonPublicParamInConstructor('Herb')
        self.assertEqual(non_pub_param_in_ctr.m_name, 'Herb')

        # Check if the main class and the nested class can be created w/o errors
        mc = binding.MainClass()
        nc = binding.MainClass.NestedClass()

        # MainClass shouldn't be a module
        self.assertFalse(inspect.ismodule(binding.MainClass))
        self.assertTrue(inspect.isclass(binding.MainClass))
        self.assertTrue(inspect.isclass(binding.MainClass.NestedClass))

        self.assertFalse(hasattr(binding, 'detail'))

        # Call static method
        self.assertEqual(binding.test1.Integer.Add(1, 2), 3)
        
        # Call instance method
        i = binding.test1.Integer(1)
        self.assertEqual(i.add(2), 3)

    def test_class(self):
        self._test_class(boost)
        if has_pybind11:
            self._test_class(py11)

if __name__ == '__main__':
    unittest.main()
