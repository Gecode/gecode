/* Boost interval/rounded_arith.hpp template implementation file
 *
 * Copyright 2002-2003 Hervé Brönnimann, Guillaume Melquiond, Sylvain Pion
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_ARITH_HPP
#define GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_ARITH_HPP

#include <gecode/third-party/boost/numeric/interval/rounding.hpp>
#include <gecode/third-party/boost/numeric/interval/detail/bugs.hpp>
#include <gecode/third-party/boost/config/no_tr1/cmath.hpp>

namespace gecode_boost {
namespace numeric {
namespace interval_lib {

/*
 * Three classes of rounding: exact, std, opp
 * See documentation for details.
 */

template<class T, class Rounding>
struct rounded_arith_exact: Rounding {
  void init() { }
  template<class U> T conv_down(U const &v) { return v; }
  template<class U> T conv_up  (U const &v) { return v; }
  T add_down (const T& x, const T& y) { return x + y; }
  T add_up   (const T& x, const T& y) { return x + y; }
  T sub_down (const T& x, const T& y) { return x - y; }
  T sub_up   (const T& x, const T& y) { return x - y; }
  T mul_down (const T& x, const T& y) { return x * y; }
  T mul_up   (const T& x, const T& y) { return x * y; }
  T div_down (const T& x, const T& y) { return x / y; }
  T div_up   (const T& x, const T& y) { return x / y; }
  T median   (const T& x, const T& y) { return (x + y) / 2; }
  T sqrt_down(const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); return sqrt(x); }
  T sqrt_up  (const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); return sqrt(x); }
  T int_down (const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(floor); return floor(x); }
  T int_up   (const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(ceil); return ceil(x); }
};

template<class T, class Rounding>
struct rounded_arith_std: Rounding {
# define GECODE_BOOST_DN(EXPR) this->downward();   return this->force_rounding(EXPR)
# define GECODE_BOOST_NR(EXPR) this->to_nearest(); return this->force_rounding(EXPR)
# define GECODE_BOOST_UP(EXPR) this->upward();     return this->force_rounding(EXPR)
  void init() { }
  template<class U> T conv_down(U const &v) { GECODE_BOOST_DN(v); }
  template<class U> T conv_up  (U const &v) { GECODE_BOOST_UP(v); }
  T add_down(const T& x, const T& y) { GECODE_BOOST_DN(x + y); }
  T sub_down(const T& x, const T& y) { GECODE_BOOST_DN(x - y); }
  T mul_down(const T& x, const T& y) { GECODE_BOOST_DN(x * y); }
  T div_down(const T& x, const T& y) { GECODE_BOOST_DN(x / y); }
  T add_up  (const T& x, const T& y) { GECODE_BOOST_UP(x + y); }
  T sub_up  (const T& x, const T& y) { GECODE_BOOST_UP(x - y); }
  T mul_up  (const T& x, const T& y) { GECODE_BOOST_UP(x * y); }
  T div_up  (const T& x, const T& y) { GECODE_BOOST_UP(x / y); }
  T median(const T& x, const T& y) { GECODE_BOOST_NR((x + y) / 2); }
  T sqrt_down(const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_DN(sqrt(x)); }
  T sqrt_up  (const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_UP(sqrt(x)); }
  T int_down(const T& x) { this->downward(); return this->to_int(x); }
  T int_up  (const T& x) { this->upward();   return this->to_int(x); }
# undef GECODE_BOOST_DN
# undef GECODE_BOOST_NR
# undef GECODE_BOOST_UP
};

template<class T, class Rounding>
struct rounded_arith_opp: Rounding {
  void init() { this->upward(); }
# define GECODE_BOOST_DN(EXPR) \
    this->downward(); \
    T r = this->force_rounding(EXPR); \
    this->upward(); \
    return r
# define GECODE_BOOST_NR(EXPR) \
    this->to_nearest(); \
    T r = this->force_rounding(EXPR); \
    this->upward(); \
    return r
# define GECODE_BOOST_UP(EXPR) return this->force_rounding(EXPR)
# define GECODE_BOOST_UP_NEG(EXPR) return -this->force_rounding(EXPR)
  template<class U> T conv_down(U const &v) { GECODE_BOOST_UP_NEG(-v); }
  template<class U> T conv_up  (U const &v) { GECODE_BOOST_UP(v); }
  // Gecode: the negated operand passes through negate_opaque.  Otherwise
  // the compiler may fuse the negation into the rounded operation, which is
  // only valid under round-to-nearest.  For example, clang on AArch64 turns
  // x * (-y) into fnmul, which negates after rounding, so mul_down returned
  // the upward-rounded product.  MSVC 2015 and 2017 on x64 did the same
  // for x * (-y) and x / (-y).
  T negate_opaque(const T& x) {
#if defined(__GNUC__) && defined(__aarch64__)
    // An empty asm hides the value without a memory round trip.
    T r = -x;
    __asm__("" : "+w"(r));
    return r;
#elif defined(__GNUC__) && defined(__x86_64__)
    // The same barrier on an SSE register.
    T r = -x;
    __asm__("" : "+x"(r));
    return r;
#else
    // Elsewhere, MSVC included (no inline asm on x64), the volatile round
    // trip is the barrier.
    return this->force_rounding(-x);
#endif
  }
  T add_down(const T& x, const T& y)
  { GECODE_BOOST_UP_NEG(negate_opaque(x) - y); }
  T sub_down(const T& x, const T& y) { GECODE_BOOST_UP_NEG(y - x); }
  T mul_down(const T& x, const T& y)
  { GECODE_BOOST_UP_NEG(x * negate_opaque(y)); }
  T div_down(const T& x, const T& y)
  { GECODE_BOOST_UP_NEG(x / negate_opaque(y)); }
  T add_up  (const T& x, const T& y) { GECODE_BOOST_UP(x + y); }
  T sub_up  (const T& x, const T& y) { GECODE_BOOST_UP(x - y); }
  T mul_up  (const T& x, const T& y) { GECODE_BOOST_UP(x * y); }
  T div_up  (const T& x, const T& y) { GECODE_BOOST_UP(x / y); }
  T median  (const T& x, const T& y) { GECODE_BOOST_NR((x + y) / 2); }
  T sqrt_down(const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_DN(sqrt(x)); }
  T sqrt_up  (const T& x)
  { GECODE_BOOST_NUMERIC_INTERVAL_using_math(sqrt); GECODE_BOOST_UP(sqrt(x)); }
  T int_down(const T& x) { return -this->to_int(-x); }
  T int_up  (const T& x) { return this->to_int(x); }
# undef GECODE_BOOST_DN
# undef GECODE_BOOST_NR
# undef GECODE_BOOST_UP
# undef GECODE_BOOST_UP_NEG
};

} // namespace interval_lib
} // namespace numeric
} // namespace gecode_boost

#endif // GECODE_BOOST_NUMERIC_INTERVAL_ROUNDED_ARITH_HPP
