namespace dart {
namespace common {

namespace signal {
namespace detail {

template <typename T>
struct DefaultCombiner
{
};

} // namespace detail
} // namespace signal

template <typename _Signature,
          template<class> class Combiner = signal::detail::DefaultCombiner>
          class Signal;


template <typename... _ArgTypes>
class Signal<void(_ArgTypes...)>
{
public:
  void f();
};

template <typename T>
class SlotRegister
{
};

}  // namespace common

namespace dynamics {

class Frame
{
};

class Entity
{
  using FrameChangedSignal
        = common::Signal<void(const Entity*,
                         const Frame* _oldFrame,
                         const Frame* _newFrame)>;

  FrameChangedSignal mFrameChangedSignal;
};

}  // namespace dynamics
}  // namespace dart

int main(int argc, char **argv)
{
  using namespace dart::dynamics;

}
