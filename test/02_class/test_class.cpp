#include <string>

namespace test_class {
namespace test_nested_class {

struct Pet
{
  Pet(const std::string &name) : name(name) { }
  void setName(const std::string &name_) { name = name_; }
  const std::string &getName() const { return name; }

  std::string name;
};

} // namespace test_nested_class
} // namespace test_class
