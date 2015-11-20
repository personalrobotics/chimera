#include <string>
#include <vector>

namespace ns {

class B {
  int x;
};

template <typename T>
class A {
  T y;
};

} // namespace

int main(int argc, char **argv)
{
  using ns::A;
  using ns::B;

  A<B> z1;
  A<std::string> z2;
  A<std::vector<B> > z3;
}
