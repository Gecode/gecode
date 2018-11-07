/* Boost interval/hw_rounding.hpp template implementation file
 *
 * Copyright 2002 Hervé Brönnimann, Guillaume Melquiond, Sylvain Pion
 * Copyright 2005 Guillaume Melquiond
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_HW_ROUNDING_HPP
#define GECODE_BOOST_NUMERIC_INTERVAL_HW_ROUNDING_HPP

#include <gecode/third-party/boost/numeric/interval/rounding.hpp>
#include <gecode/third-party/boost/numeric/interval/rounded_arith.hpp>

#define GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(__cplusplus) && __cplusplus >= 201103L)
#  define GECODE_BOOST_C99_ENABLED
#endif

// define appropriate specialization of rounding_control for built-in types
#if defined(__x86_64__) && (defined(GECODE_BOOST_C99_ENABLED) || defined(__APPLE__))
#  include <gecode/third-party/boost/numeric/interval/detail/c99_rounding_control.hpp>
#elif defined(__i386__) || defined(_M_IX86) || defined(__BORLANDC__) || defined(_M_X64)
#  include <gecode/third-party/boost/numeric/interval/detail/x86_rounding_control.hpp>
#elif defined(powerpc) || defined(__powerpc__) || defined(__ppc__)
#ifndef __NO_FPRS__
#  include <gecode/third-party/boost/numeric/interval/detail/ppc_rounding_control.hpp>
#endif
#elif defined(sparc) || defined(__sparc__)
#  include <gecode/third-party/boost/numeric/interval/detail/sparc_rounding_control.hpp>
#elif defined(alpha) || defined(__alpha__)
#  include <gecode/third-party/boost/numeric/interval/detail/alpha_rounding_control.hpp>
#elif defined(ia64) || defined(__ia64) || defined(__ia64__)
#  include <gecode/third-party/boost/numeric/interval/detail/ia64_rounding_control.hpp>
#endif

#if defined(GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE) && (defined(GECODE_BOOST_C99_ENABLED) || defined(__MSL__))
#  include <gecode/third-party/boost/numeric/interval/detail/c99_rounding_control.hpp>
#endif

#if defined(GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE)
#  undef GECODE_BOOST_NUMERIC_INTERVAL_NO_HARDWARE
#  error Boost.Numeric.Interval: Please specify rounding control mechanism.
#endif

#undef GECODE_BOOST_C99_ENABLED

namespace gecode_boost {
namespace numeric {
namespace interval_lib {

/*
 * Three specializations of rounded_math<T>
 */

template<>
struct rounded_math<float>
  : save_state<rounded_arith_opp<float> >
{};

template<>
struct rounded_math<double>
  : save_state<rounded_arith_opp<double> >
{};

template<>
struct rounded_math<long double>
  : save_state<rounded_arith_opp<long double> >
{};

} // namespace interval_lib
} // namespace numeric
} // namespace gecode_boost

#endif // GECODE_BOOST_NUMERIC_INTERVAL_HW_ROUNDING_HPP
