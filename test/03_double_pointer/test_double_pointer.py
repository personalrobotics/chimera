import unittest

from test_double_pointer_boost_python.nested import Node as Node_bp


class TestClass(unittest.TestCase):
    def test_name_pb(self):
        parent = Node_bp()
        child = Node_bp(parent)
        assert child.mParent == parent


if __name__ == '__main__':
    unittest.main()
