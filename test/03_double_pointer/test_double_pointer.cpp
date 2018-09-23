#include <string>

namespace test_double_pointer {
namespace nested {

struct Node
{
  Node* mParent;
  unsigned int mNumChildren;
  Node** mChildren;

  Node()
  {
    mParent = nullptr;
    mNumChildren = 0;
    mChildren = nullptr;
  }

  Node(Node* parent)
  {
    mParent = parent;
    mNumChildren = 0;
    mChildren = nullptr;

    if (mParent)
      mParent->addChild(this);
  }

  ~Node()
  {
    if (mChildren && mNumChildren)
    {
      for (unsigned int i = 0; i < mNumChildren; i++)
        delete mChildren[i];
    }
    delete [] mChildren;
  }

  void addChild(Node* node)
  {
    if (!node)
      return;

    Node** newChildren = new Node*[mNumChildren + 1];
    if (mChildren)
    {
      for (unsigned int i = 0; i < mNumChildren; ++i)
        newChildren[i] = mChildren[i];
    }
    newChildren[mNumChildren] = node;

    mChildren = newChildren;
    mNumChildren++;
  }
};

} // namespace nested
} // namespace test_double_pointer
