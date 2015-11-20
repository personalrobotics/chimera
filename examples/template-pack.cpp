#include <vector>

namespace ns {

class C {
};

template <typename T>
class B {
};

template <typename ... T>
class A {
};

} // namespace ns

int main(int argc, char **argv)
{
  using namespace ns;
  A<C> x1;
  A<B<C> > x2;
}
