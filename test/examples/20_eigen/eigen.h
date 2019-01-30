#include <iostream>
#include <Eigen/Dense>

namespace chimera_test {
namespace nested_namespace {

template <typename Derived>
void print_size(const Eigen::EigenBase<Derived>& b)
{
  std::cout << "size (rows, cols): " << b.size() << " (" << b.rows()
            << ", " << b.cols() << ")" << std::endl;
}

inline void test_print_size()
{
  print_size(Eigen::Matrix3d::Identity());
}

template <typename Derived>
void print_block(const Eigen::DenseBase<Derived>& b, int x, int y, int r, int c)
{
  std::cout << "block: " << b.block(x,y,r,c) << std::endl;
}

inline void test_print_block()
{
  print_block(Eigen::Matrix3d::Identity(), 1, 2, 2, 1);
}

template <typename Derived>
void print_max_coeff(const Eigen::ArrayBase<Derived> &a)
{
  std::cout << "max: " << a.maxCoeff() << std::endl;
}

inline void test_print_max_coeff()
{
  print_max_coeff(Eigen::ArrayXd::Zero(3));
}

template <typename Derived>
void print_inv_cond(const Eigen::MatrixBase<Derived>& a)
{
  const typename Eigen::JacobiSVD<typename Derived::PlainObject>::SingularValuesType&
    sing_vals = a.jacobiSvd().singularValues();
  std::cout << "inv cond: " << sing_vals(sing_vals.size()-1) / sing_vals(0) << std::endl;
}

inline void test_print_inv_cond()
{
  print_inv_cond(Eigen::Matrix3d::Identity());
}

void function_with_map_param(Eigen::Map<Eigen::VectorXd> map);

void test_function_with_map_param();

} // namespace nested_namespace
} // namespace chimera_test
