/* Boost interval/detail/c99_rounding_control.hpp file
 *
 * Copyright 2000 Jens Maurer
 * Copyright 2002 Hervé Brönnimann, Guillaume Melquiond, Sylvain Pion
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_C99_ROUNDING_CONTROL_HPP
#define GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_C99_ROUNDING_CONTROL_HPP

#include <gecode/third-party/boost/numeric/interval/detail/c99sub_rounding_control.hpp>

namespace gecode_boost {
namespace numeric {
namespace interval_lib {

template<>
struct rounding_control<float>:
  detail::c99_rounding_control
{
  static float force_rounding(float const &r)
  { volatile float r_ = r; return r_; }
};

template<>
struct rounding_control<double>:
  detail::c99_rounding_control
{
  static double force_rounding(double const &r)
  { volatile double r_ = r; return r_; }
};

template<>
struct rounding_control<long double>:
  detail::c99_rounding_control
{
  static long double force_rounding(long double const &r)
  { volatile long double r_ = r; return r_; }
};

} // namespace interval_lib
} // namespace numeric
} // namespace gecode_boost

#undef GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE

#endif // GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_C99_ROUNDING_CONTROL_HPP
