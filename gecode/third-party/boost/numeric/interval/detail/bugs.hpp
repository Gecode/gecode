/* Boost interval/detail/bugs.hpp file
 *
 * Copyright 2000 Jens Maurer
 * Copyright 2002 Hervé Brönnimann, Guillaume Melquiond, Sylvain Pion
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_BUGS
#define GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_BUGS

#include <gecode/third-party/boost/config.hpp>

#if defined(__GLIBC__) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_ISOC99)) && !defined(__ICC)
#  define GECODE_BOOST_HAS_INV_HYPERBOLIC
#endif

#ifdef GECODE_BOOST_NO_STDC_NAMESPACE
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_math(a) using ::a
#  ifdef GECODE_BOOST_HAS_INV_HYPERBOLIC
#    define GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(a) using ::a
#  endif
#else
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_math(a) using std::a
#  if defined(GECODE_BOOST_HAS_INV_HYPERBOLIC)
#    if defined(__GLIBCPP__) || defined(__GLIBCXX__)
#    define GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(a) using ::a
#    else
#    define GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(a) using std::a
#  endif
#  endif
#endif

#if defined(__COMO__) || defined(GECODE_BOOST_INTEL)
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_max(a) using std::a
#elif defined(GECODE_BOOST_NO_STDC_NAMESPACE)
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_max(a) using ::a
#else
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_max(a) using std::a
#endif

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp
#  define GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(a)
#endif

#if defined(__GNUC__) && (__GNUC__ <= 2)
// cf PR c++/1981 for a description of the bug
#include <algorithm>
#include <gecode/third-party/boost/config/no_tr1/cmath.hpp>
namespace gecode_boost {
namespace numeric {
  using std::min;
  using std::max;
  using std::sqrt;
  using std::exp;
  using std::log;
  using std::cos;
  using std::tan;
  using std::asin;
  using std::acos;
  using std::atan;
  using std::ceil;
  using std::floor;
  using std::sinh;
  using std::cosh;
  using std::tanh;
# undef GECODE_BOOST_NUMERIC_INTERVAL_using_max
# undef GECODE_BOOST_NUMERIC_INTERVAL_using_math
# define GECODE_BOOST_NUMERIC_INTERVAL_using_max(a)
# define GECODE_BOOST_NUMERIC_INTERVAL_using_math(a)
# undef GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp
# define GECODE_BOOST_NUMERIC_INTERVAL_using_ahyp(a)
} // namespace numeric
} // namespace gecode_boost
#endif

#endif // GECODE_BOOST_NUMERIC_INTERVAL_DETAIL_BUGS
