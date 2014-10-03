//  (C) Copyright John Maddock 2001 - 2002. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

#if __IBMCPP__ <= 501
#  define GECODE_BOOST_NO_STD_ALLOCATOR
#endif

#define GECODE_BOOST_HAS_MACRO_USE_FACET
#define GECODE_BOOST_NO_STD_MESSAGES

// Apple doesn't seem to reliably defined a *unix* macro
#if !defined(CYGWIN) && (  defined(__unix__)  \
                        || defined(__unix)    \
                        || defined(unix)      \
                        || defined(__APPLE__) \
                        || defined(__APPLE)   \
                        || defined(APPLE))
#  include <unistd.h>
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

#define GECODE_BOOST_STDLIB "Visual Age default standard library"



