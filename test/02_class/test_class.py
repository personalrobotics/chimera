import unittest

try:
    from test_class_pybind11.test_nested_class import Pet as Pet_py11
    has_pybind11 = True
except:
    has_pybind11 = False

from test_class_boost_python.test_nested_class import Pet as Pet_bp


class TestClass(unittest.TestCase):

    def test_name_py11(self):
        if not has_pybind11:
            return

        p = Pet_py11('Molly')
        self.assertEqual(p.name, 'Molly')
        self.assertEqual(p.getName(), 'Molly')

        p.setName('Charly')
        self.assertEqual(p.name, 'Charly')
        self.assertEqual(p.getName(), 'Charly')

    def test_name_pb(self):
        p = Pet_bp('Molly')
        self.assertEqual(p.name, 'Molly')
        self.assertEqual(p.getName(), 'Molly')

        p.setName('Charly')
        self.assertEqual(p.name, 'Charly')
        self.assertEqual(p.getName(), 'Charly')


if __name__ == '__main__':
    unittest.main()
