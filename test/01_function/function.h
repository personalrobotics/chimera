namespace function {
namespace nested_function {

int add(int i = 1, int j = 2);

inline int inline_add(int i = 1, int j = 2)
{
  return i + j;
}

void void_return();

} // namespace nested_function
} // namespace function
