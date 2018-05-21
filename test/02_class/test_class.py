import unittest

from test_class_pybind11.test_nested_class import Pet as Pet_py11
from test_class_boost_python.test_nested_class import Pet as Pet_bp


class TestClass(unittest.TestCase):

    def test_name_py11(self):
        p = Pet_py11('Molly')
        self.assertEqual(p.name, 'Molly')

        p.setName('Charly')
        self.assertEqual(p.name, 'Charly')

    def test_name_pb(self):
        p = Pet_bp('Molly')
        self.assertEqual(p.name, 'Molly')

        p.setName('Charly')
        self.assertEqual(p.name, 'Charly')


if __name__ == '__main__':
    unittest.main()
