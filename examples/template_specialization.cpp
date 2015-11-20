#include <string>
#include <vector>

namespace ns {

class B {
};

template <typename T>
class A {
};

template <typename... Args>
class A<void (Args...)> {
};

} // namespace

int main(int argc, char **argv)
{
  using namespace ns;
  A<void (B)> x;
}
