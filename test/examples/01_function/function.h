#pragma once

#include <memory>

namespace chimera_test
{

class A
{
};

void void_bool(bool);
void void_int(int);
void void_long(long);
void void_float(float);
void void_double(double);
void void_a(A);
void void_shared_a(std::shared_ptr<A>);
void void_pair(std::pair<int, int>);

// void void_ref_bool(bool &)   {}
// void void_ref_int(int &)    {}
// void void_ref_long(long &)   {}
// void void_ref_float(float &)  {}
// void void_ref_double(double &) {}
// void void_ref_a(A &)      {}
// void void_ref_shared_a(std::shared_ptr<A> &) {}
// void void_ref_pair_int_int(std::pair<int, int> &) {}

void void_cr_bool(const bool &);
void void_cr_int(const int &);
void void_cr_long(const long &);
void void_cr_float(const float &);
void void_cr_double(const double &);
void void_cr_a(const A &);
void void_cr_shared_a(const std::shared_ptr<A> &);
void void_cr_shared_c_a(const std::shared_ptr<A const> &);
void void_cr_pair(const std::pair<int, int> &);

void void_p_bool(bool *);
void void_p_int(int *);
void void_p_long(long *);
void void_p_float(float *);
void void_p_double(double *);
void void_p_a(A *);
void void_p_shared_a(std::shared_ptr<A> *);
// void void_p_pair(std::pair<int, int> *);

int add(int i = 1, int j = 2);

inline int inline_add(int i = 1, int j = 2)
{
    return i + j;
}

struct Dummy
{
    int val;
    Dummy() = default;
};

int void_pointer_param(void *dummy);

void void_param(void);

void void_return();

// This class is suppressed by the configuration so that not to be binded
class SuppressedClass
{
};

// This function should be suppressed as well because the parameter is of a
// suppressed type.
void function_with_suppressed_param(const SuppressedClass &);

// This class is suppressed by the configuration so that not to be binded
template <typename T>
class SuppressedTemplateClass
{
};

// This function should be suppressed as well because the parameter is of a
// suppressed type.
void function_with_suppressed_template_param(
    const SuppressedTemplateClass<int> &);

} // namespace chimera_test
