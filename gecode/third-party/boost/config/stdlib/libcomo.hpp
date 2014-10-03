//  (C) Copyright John Maddock 2002 - 2003. 
//  (C) Copyright Jens Maurer 2002 - 2003. 
//  (C) Copyright Beman Dawes 2002 - 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Comeau STL:

#if !defined(__LIBCOMO__)
#  include <gecode/third-party/boost/config/no_tr1/utility.hpp>
#  if !defined(__LIBCOMO__)
#      error "This is not the Comeau STL!"
#  endif
#endif

//
// std::streambuf<wchar_t> is non-standard
// NOTE: versions of libcomo prior to beta28 have octal version numbering,
// e.g. version 25 is 21 (dec)
#if __LIBCOMO_VERSION__ <= 22
#  define GECODE_BOOST_NO_STD_WSTREAMBUF
#endif

#if (__LIBCOMO_VERSION__ <= 31) && defined(_WIN32)
#define GECODE_BOOST_NO_SWPRINTF
#endif

#if __LIBCOMO_VERSION__ >= 31
#  define GECODE_BOOST_HAS_HASH
#  define GECODE_BOOST_HAS_SLIST
#endif

//  C++0x headers not yet implemented
//
#  define GECODE_BOOST_NO_0X_HDR_ARRAY
#  define GECODE_BOOST_NO_0X_HDR_CHRONO
#  define GECODE_BOOST_NO_0X_HDR_CODECVT
#  define GECODE_BOOST_NO_0X_HDR_CONDITION_VARIABLE
#  define GECODE_BOOST_NO_0X_HDR_FORWARD_LIST
#  define GECODE_BOOST_NO_0X_HDR_FUTURE
#  define GECODE_BOOST_NO_0X_HDR_INITIALIZER_LIST
#  define GECODE_BOOST_NO_0X_HDR_MUTEX
#  define GECODE_BOOST_NO_0X_HDR_RANDOM
#  define GECODE_BOOST_NO_0X_HDR_RATIO
#  define GECODE_BOOST_NO_0X_HDR_REGEX
#  define GECODE_BOOST_NO_0X_HDR_SYSTEM_ERROR
#  define GECODE_BOOST_NO_0X_HDR_THREAD
#  define GECODE_BOOST_NO_0X_HDR_TUPLE
#  define GECODE_BOOST_NO_0X_HDR_TYPE_TRAITS
#  define GECODE_BOOST_NO_0X_HDR_TYPEINDEX
#  define GECODE_BOOST_NO_STD_UNORDERED        // deprecated; see following
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_MAP
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_SET
#  define GECODE_BOOST_NO_NUMERIC_LIMITS_LOWEST

//
// Intrinsic type_traits support.
// The SGI STL has it's own __type_traits class, which
// has intrinsic compiler support with SGI's compilers.
// Whatever map SGI style type traits to boost equivalents:
//
#define GECODE_BOOST_HAS_SGI_TYPE_TRAITS

#define GECODE_BOOST_STDLIB "Comeau standard library " GECODE_BOOST_STRINGIZE(__LIBCOMO_VERSION__)


