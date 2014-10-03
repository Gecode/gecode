//  (C) Copyright John Maddock 2001. 
//  (C) Copyright Darin Adler 2001. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Metrowerks standard library:

#ifndef __MSL_CPP__
#  include <gecode/third-party/boost/config/no_tr1/utility.hpp>
#  ifndef __MSL_CPP__
#     error This is not the MSL standard library!
#  endif
#endif

#if __MSL_CPP__ >= 0x6000  // Pro 6
#  define GECODE_BOOST_HAS_HASH
#  define GECODE_BOOST_STD_EXTENSION_NAMESPACE Metrowerks
#endif
#define GECODE_BOOST_HAS_SLIST

#if __MSL_CPP__ < 0x6209
#  define GECODE_BOOST_NO_STD_MESSAGES
#endif

// check C lib version for <stdint.h>
#include <cstddef>

#if defined(__MSL__) && (__MSL__ >= 0x5000)
#  define GECODE_BOOST_HAS_STDINT_H
#  if !defined(__PALMOS_TRAPS__)
#    define GECODE_BOOST_HAS_UNISTD_H
#  endif
   // boilerplate code:
#  include <gecode/third-party/boost/config/posix_features.hpp>
#endif

#if defined(_MWMT) || _MSL_THREADSAFE
#  define GECODE_BOOST_HAS_THREADS
#endif

#ifdef _MSL_NO_EXPLICIT_FUNC_TEMPLATE_ARG
#  define GECODE_BOOST_NO_STD_USE_FACET
#  define GECODE_BOOST_HAS_TWO_ARG_USE_FACET
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

#define GECODE_BOOST_STDLIB "Metrowerks Standard Library version " GECODE_BOOST_STRINGIZE(__MSL_CPP__)









