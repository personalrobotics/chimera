import unittest

try:
    import class_pybind11.nested_namespace as py11
    has_pybind11 = True
except:
    has_pybind11 = False

import class_boost_python.nested_namespace as boost


class TestFunction(unittest.TestCase):

    def test_function_py11(self):
        if not has_pybind11:
            return

        dog = py11.Dog()
        self.assertEqual(dog.type(), 'Dog')
        self.assertEqual(dog.pure_virtual_type(), 'Dog')
        self.assertEqual(py11.Dog.static_type(), 'Dog')

        husky = py11.Husky()
        self.assertEqual(husky.type(), 'Husky')
        self.assertEqual(husky.pure_virtual_type(), 'Husky')

        strong_husky = py11.StrongHusky()
        self.assertEqual(strong_husky.type(), 'StrongHusky')
        self.assertEqual(strong_husky.pure_virtual_type(), 'StrongHusky')

        default_args = py11.DefaultArguments()
        self.assertEqual(default_args.add(), 3)
        self.assertEqual(default_args.add(2), 4)
        self.assertEqual(default_args.add(2, 3), 5)

        self.assertEqual(py11.StaticFields.m_static_readonly_type, 'static readonly type')
        self.assertEqual(py11.StaticFields.m_static_readwrite_type, 'static readwrite type')
        py11.StaticFields.m_static_readwrite_type = 'new type'
        self.assertEqual(py11.StaticFields.m_static_readwrite_type, 'new type')
        self.assertEqual(py11.StaticFields.static_type(), 'static type')

        non_pub_param_in_ctr = py11.NonPublicParamInConstructor('Herb')
        self.assertEqual(non_pub_param_in_ctr.m_name, 'Herb')

    def test_function_bp(self):
        dog = boost.Dog()
        self.assertEqual(dog.type(), 'Dog')
        self.assertEqual(dog.pure_virtual_type(), 'Dog')
        self.assertEqual(boost.Dog.static_type(), 'Dog')

        husky = boost.Husky()
        self.assertEqual(husky.type(), 'Husky')
        self.assertEqual(husky.pure_virtual_type(), 'Husky')

        strong_husky = boost.StrongHusky()
        self.assertEqual(strong_husky.type(), 'StrongHusky')
        self.assertEqual(strong_husky.pure_virtual_type(), 'StrongHusky')

        default_args = boost.DefaultArguments()
        self.assertEqual(default_args.add(), 3)
        self.assertEqual(default_args.add(2), 4)
        self.assertEqual(default_args.add(2, 3), 5)

        self.assertEqual(boost.StaticFields.m_static_readonly_type, 'static readonly type')
        self.assertEqual(boost.StaticFields.m_static_readwrite_type, 'static readwrite type')
        # TODO(JS): Boost.Python doesn't work for readwrite static field (see: #194)
        # boost.StaticFields.m_static_readwrite_type = 'new type'
        # self.assertEqual(boost.StaticFields.m_static_readwrite_type, 'new type')
        self.assertEqual(boost.StaticFields.static_type(), 'static type')

        non_pub_param_in_ctr = boost.NonPublicParamInConstructor('Herb')
        self.assertEqual(non_pub_param_in_ctr.m_name, 'Herb')


if __name__ == '__main__':
    unittest.main()
