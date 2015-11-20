namespace ns {

template <typename T>
class B {
  T x;
};

template <template <typename> class T>
class A
{
  T<int> y;
};

} // namespace

int main(int argc, char **argv)
{
  using ns::A;
  using ns::B;

  A<B> z;
}
