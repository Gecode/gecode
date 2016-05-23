
//  (C) Copyright John maddock 1999.
//  (C) David Abrahams 2002.  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// use this header as a workaround for missing <limits>

//  See http://www.boost.org/libs/compatibility/index.html for documentation.

#ifndef GECODE_BOOST_LIMITS
#define GECODE_BOOST_LIMITS

#include <gecode/third-party/boost/config.hpp>

#ifdef GECODE_BOOST_NO_LIMITS
# include <gecode/third-party/boost/detail/limits.hpp>
#else
# include <limits>
#endif

#if (defined(GECODE_BOOST_HAS_LONG_LONG) && defined(GECODE_BOOST_NO_LONG_LONG_NUMERIC_LIMITS)) \
      || (defined(GECODE_BOOST_HAS_MS_INT64) && defined(GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS))
// Add missing specializations for numeric_limits:
#ifdef GECODE_BOOST_HAS_MS_INT64
#  define GECODE_BOOST_LLT __int64
#  define GECODE_BOOST_ULLT unsigned __int64
#else
#  define GECODE_BOOST_LLT  ::gecode_boost::long_long_type
#  define GECODE_BOOST_ULLT  ::gecode_boost::ulong_long_type
#endif

#include <climits>  // for CHAR_BIT

namespace std
{
  template<>
  class numeric_limits<GECODE_BOOST_LLT>
  {
   public:

      GECODE_BOOST_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef GECODE_BOOST_HAS_MS_INT64
      static GECODE_BOOST_LLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 0x8000000000000000i64; }
      static GECODE_BOOST_LLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 0x7FFFFFFFFFFFFFFFi64; }
#elif defined(LLONG_MAX)
      static GECODE_BOOST_LLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MIN; }
      static GECODE_BOOST_LLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MAX; }
#elif defined(LONGLONG_MAX)
      static GECODE_BOOST_LLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MIN; }
      static GECODE_BOOST_LLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MAX; }
#else
      static GECODE_BOOST_LLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 1LL << (sizeof(GECODE_BOOST_LLT) * CHAR_BIT - 1); }
      static GECODE_BOOST_LLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ~(min)(); }
#endif
      GECODE_BOOST_STATIC_CONSTANT(int, digits = sizeof(GECODE_BOOST_LLT) * CHAR_BIT -1);
      GECODE_BOOST_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (GECODE_BOOST_LLT) - 1) * 301L / 1000);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_signed = true);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_integer = true);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_exact = true);
      GECODE_BOOST_STATIC_CONSTANT(int, radix = 2);
      static GECODE_BOOST_LLT epsilon() throw() { return 0; };
      static GECODE_BOOST_LLT round_error() throw() { return 0; };

      GECODE_BOOST_STATIC_CONSTANT(int, min_exponent = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, min_exponent10 = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, max_exponent = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, max_exponent10 = 0);

      GECODE_BOOST_STATIC_CONSTANT(bool, has_infinity = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_denorm = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static GECODE_BOOST_LLT infinity() throw() { return 0; };
      static GECODE_BOOST_LLT quiet_NaN() throw() { return 0; };
      static GECODE_BOOST_LLT signaling_NaN() throw() { return 0; };
      static GECODE_BOOST_LLT denorm_min() throw() { return 0; };

      GECODE_BOOST_STATIC_CONSTANT(bool, is_iec559 = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_bounded = true);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_modulo = true);

      GECODE_BOOST_STATIC_CONSTANT(bool, traps = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, tinyness_before = false);
      GECODE_BOOST_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);

  };

  template<>
  class numeric_limits<GECODE_BOOST_ULLT>
  {
   public:

      GECODE_BOOST_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef GECODE_BOOST_HAS_MS_INT64
      static GECODE_BOOST_ULLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 0ui64; }
      static GECODE_BOOST_ULLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 0xFFFFFFFFFFFFFFFFui64; }
#elif defined(ULLONG_MAX) && defined(ULLONG_MIN)
      static GECODE_BOOST_ULLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MIN; }
      static GECODE_BOOST_ULLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MAX; }
#elif defined(ULONGLONG_MAX) && defined(ULONGLONG_MIN)
      static GECODE_BOOST_ULLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MIN; }
      static GECODE_BOOST_ULLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MAX; }
#else
      static GECODE_BOOST_ULLT min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return 0uLL; }
      static GECODE_BOOST_ULLT max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (){ return ~0uLL; }
#endif
      GECODE_BOOST_STATIC_CONSTANT(int, digits = sizeof(GECODE_BOOST_LLT) * CHAR_BIT);
      GECODE_BOOST_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (GECODE_BOOST_LLT)) * 301L / 1000);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_signed = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_integer = true);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_exact = true);
      GECODE_BOOST_STATIC_CONSTANT(int, radix = 2);
      static GECODE_BOOST_ULLT epsilon() throw() { return 0; };
      static GECODE_BOOST_ULLT round_error() throw() { return 0; };

      GECODE_BOOST_STATIC_CONSTANT(int, min_exponent = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, min_exponent10 = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, max_exponent = 0);
      GECODE_BOOST_STATIC_CONSTANT(int, max_exponent10 = 0);

      GECODE_BOOST_STATIC_CONSTANT(bool, has_infinity = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_denorm = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static GECODE_BOOST_ULLT infinity() throw() { return 0; };
      static GECODE_BOOST_ULLT quiet_NaN() throw() { return 0; };
      static GECODE_BOOST_ULLT signaling_NaN() throw() { return 0; };
      static GECODE_BOOST_ULLT denorm_min() throw() { return 0; };

      GECODE_BOOST_STATIC_CONSTANT(bool, is_iec559 = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_bounded = true);
      GECODE_BOOST_STATIC_CONSTANT(bool, is_modulo = true);

      GECODE_BOOST_STATIC_CONSTANT(bool, traps = false);
      GECODE_BOOST_STATIC_CONSTANT(bool, tinyness_before = false);
      GECODE_BOOST_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);

  };
}
#endif

#endif

