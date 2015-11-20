namespace ns {

template <typename T>
class B {
  T x;
};

template <template <typename> class T>
class A {
public:
  int bar;

  void foo();
};

class Banana {
public:
  static A<B> baz;
};

void f(A<B> x)
{
}

} // namespace

int main(int argc, char **argv)
{
  using ns::A;
  using ns::B;

  A<B> z;
}
