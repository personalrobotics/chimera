import unittest

import object_oriented_code_pybind11 as py11
import object_oriented_code_boost_python as boost


class TestObjectOrientedCode(unittest.TestCase):

    def _test_function(self, binding):
        # Test class docstring
        self.assertTrue(
            "Pet implementation that has name" in binding.test1.Pet.__doc__)

        p = binding.test1.Pet('Molly')

        self.assertEqual(p.getName(), 'Molly')
        p.setName('Charly')
        self.assertEqual(p.getName(), 'Charly')

        self.assertEqual(p.name, 'Charly')
        p.name = 'Molly'
        self.assertEqual(p.name, 'Molly')

        # Test dynamic attributes
        if binding is py11:
            # Adding new dynamic attribute, age, to Pet
            p.age = 2
            self.assertEqual(p.age, 2)

        d = binding.test1.Dog('Molly')
        self.assertEqual(d.getName(), 'Molly')
        self.assertEqual(d.bark(), 'woof!')

        # TODO: Fix for boost
        if binding is py11:
            p2 = binding.test1.create_pet()
            self.assertFalse(hasattr(p2, 'bark'))

            p3 = binding.test1.create_polymorphic_pet()
            self.assertTrue(hasattr(p3, 'bark'))
            self.assertEqual(p3.bark(), 'woof!')

        # Test overloaded methods
        p4 = binding.test2.Pet('Molly', 2)
        self.assertEqual(p4.name, 'Molly')
        self.assertEqual(p4.age, 2)
        p4.set('Charly')  # Set name
        p4.set(4)  # Set age
        self.assertEqual(p4.name, 'Charly')
        self.assertEqual(p4.age, 4)

        # Test enumerations and internal types
        if binding is boost:
            # TODO: Fix Boost.Python to expose (classical) enum values to its
            # parent scope
            p5 = binding.test3.Pet('Lucy', binding.test3.Pet.Kind.Cat)
            self.assertEqual(p5.type, binding.test3.Pet.Kind.Cat)
            self.assertEqual(int(p5.type), 1)
        else:
            p5 = binding.test3.Pet('Lucy', binding.test3.Pet.Cat)
            self.assertEqual(p5.type, binding.test3.Pet.Cat)
            self.assertEqual(int(p5.type), 1)

    def test_function(self):
        self._test_function(py11)
        self._test_function(boost)


if __name__ == '__main__':
    unittest.main()
