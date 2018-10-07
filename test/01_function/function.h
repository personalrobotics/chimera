namespace function {
namespace nested_function {

int add(int i = 1, int j = 2);

inline int inline_add(int i = 1, int j = 2)
{
  return i + j;
}

struct Dummy {
  int val;
  Dummy() = default;
};

int void_pointer_param(void* dummy);

void void_param(void);

void void_return();

} // namespace nested_function
} // namespace function
