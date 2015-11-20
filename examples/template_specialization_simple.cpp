namespace ns {

class B { };

template <class T>
class A { };

} // namespace

int main(int argc, char **argv)
{
  using namespace ns;

  A<void (B)> x;
}
