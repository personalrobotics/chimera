import unittest

import object_oriented_code_pybind11 as py11
import object_oriented_code_boost_python as boost


class TestObjectOrientedCode(unittest.TestCase):

    def _test_function(self, binding):
        p = binding.Pet('Molly')

        self.assertEqual(p.getName(), 'Molly')
        p.setName('Charly')
        self.assertEqual(p.getName(), 'Charly')

        self.assertEqual(p.name, 'Charly')
        p.name = 'Molly'
        self.assertEqual(p.name, 'Molly')

        # Dynamic attributes
        if binding is py11:
            # Adding new dynamic attribute, age, to Pet
            p.age = 2
            self.assertEqual(p.age, 2)

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
