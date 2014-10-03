//  (C) Copyright John Maddock 2008.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// The aim of this header is just to include <cmath> but to do
// so in a way that does not result in recursive inclusion of
// the Boost TR1 components if boost/tr1/tr1/cmath is in the
// include search path.  We have to do this to avoid circular
// dependencies:
//

#ifndef GECODE_BOOST_CONFIG_CMATH
#  define GECODE_BOOST_CONFIG_CMATH

#  ifndef GECODE_BOOST_TR1_NO_RECURSION
#     define GECODE_BOOST_TR1_NO_RECURSION
#     define GECODE_BOOST_CONFIG_NO_CMATH_RECURSION
#  endif

#  include <cmath>

#  ifdef GECODE_BOOST_CONFIG_NO_CMATH_RECURSION
#     undef GECODE_BOOST_TR1_NO_RECURSION
#     undef GECODE_BOOST_CONFIG_NO_CMATH_RECURSION
#  endif

#endif
