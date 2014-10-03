/* Boost interval/rounded_transc.hpp template implementation file
 *
 * Copyright 2002-2003 Hervé Brönnimann, Guillaume Melquiond, Sylvain Pion
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_TRANSC_HPP
#define GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_TRANSC_HPP

#include <gecode/third-party/boost/numeric/interval/rounding.hpp>
#include <gecode/third-party/boost/numeric/interval/detail/bugs.hpp>
#include <gecode/third-party/boost/config/no_tr1/cmath.hpp>

namespace gecode_boost {
namespace numeric {
namespace interval_lib {

template<class T, class Rounding>
struct rounded_transc_exact: Rounding
{
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); return f(x); } \
    T f##_up  (const T& x) { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); return f(x); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(exp)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(log)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cosh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); return f(x); } \
    T f##_up  (const T& x) { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); return f(x); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acosh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
};
  
template<class T, class Rounding>
struct rounded_transc_std: Rounding
{
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      this->downward(); return this->force_rounding(f(x)); } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      this->upward(); return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(exp)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(log)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cosh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      this->downward(); return this->force_rounding(f(x)); } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      this->upward(); return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acosh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
};

template<class T, class Rounding>
struct rounded_transc_opp: Rounding
{
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      this->downward(); T y = this->force_rounding(f(x)); \
      this->upward(); return y; } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(exp)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(log)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acos)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(cosh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      return -this->force_rounding(-f(x)); } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_math(f); \
      return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asin)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atan)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(sinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(tanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      this->downward(); T y = this->force_rounding(f(x)); \
      this->upward(); return y; } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(asinh)
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(atanh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
# define GECODE_BOOST_NUMERIC_INTERVAL_new_func(f) \
    T f##_down(const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      return -this->force_rounding(-f(x)); } \
    T f##_up  (const T& x) \
    { GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(f); \
      return this->force_rounding(f(x)); }
  GECODE_BOOST_NUMERIC_INTERVAL_new_func(acosh)
# undef GECODE_BOOST_NUMERIC_INTERVAL_new_func
};
  
} // namespace interval_lib
} // namespace numeric
} // namespace gecode_boost

#endif // GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_TRANSC_HPP
