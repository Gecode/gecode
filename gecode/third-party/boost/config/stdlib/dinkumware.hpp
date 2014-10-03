//  (C) Copyright John Maddock 2001 - 2003.
//  (C) Copyright Jens Maurer 2001.
//  (C) Copyright Peter Dimov 2001.
//  (C) Copyright David Abrahams 2002.
//  (C) Copyright Guillaume Melquiond 2003.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Dinkumware standard library config:

#if !defined(_YVALS) && !defined(_CPPLIB_VER)
#include <gecode/third-party/boost/config/no_tr1/utility.hpp>
#if !defined(_YVALS) && !defined(_CPPLIB_VER)
#error This is not the Dinkumware lib!
#endif
#endif


#if defined(_CPPLIB_VER) && (_CPPLIB_VER >= 306)
   // full dinkumware 3.06 and above
   // fully conforming provided the compiler supports it:
#  if !(defined(_GLOBAL_USING) && (_GLOBAL_USING+0 > 0)) && !defined(__BORLANDC__) && !defined(_STD) && !(defined(__ICC) && (__ICC >= 700))   // can be defined in yvals.h
#     define GECODE_BOOST_NO_STDC_NAMESPACE
#  endif
#  if !(defined(_HAS_MEMBER_TEMPLATES_REBIND) && (_HAS_MEMBER_TEMPLATES_REBIND+0 > 0)) && !(defined(_MSC_VER) && (_MSC_VER > 1300)) && defined(GECODE_BOOST_MSVC)
#     define GECODE_BOOST_NO_STD_ALLOCATOR
#  endif
#  define GECODE_BOOST_HAS_PARTIAL_STD_ALLOCATOR
#  if defined(GECODE_BOOST_MSVC) && (GECODE_BOOST_MSVC < 1300)
      // if this lib version is set up for vc6 then there is no std::use_facet:
#     define GECODE_BOOST_NO_STD_USE_FACET
#     define GECODE_BOOST_HAS_TWO_ARG_USE_FACET
      // C lib functions aren't in namespace std either:
#     define GECODE_BOOST_NO_STDC_NAMESPACE
      // and nor is <exception>
#     define GECODE_BOOST_NO_EXCEPTION_STD_NAMESPACE
#  endif
// There's no numeric_limits<long long> support unless _LONGLONG is defined:
#  if !defined(_LONGLONG) && (_CPPLIB_VER <= 310)
#     define GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS
#  endif
// 3.06 appears to have (non-sgi versions of) <hash_set> & <hash_map>,
// and no <slist> at all
#else
#  define GECODE_BOOST_MSVC_STD_ITERATOR 1
#  define GECODE_BOOST_NO_STD_ITERATOR
#  define GECODE_BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
#  define GECODE_BOOST_NO_STD_ALLOCATOR
#  define GECODE_BOOST_NO_STDC_NAMESPACE
#  define GECODE_BOOST_NO_STD_USE_FACET
#  define GECODE_BOOST_NO_STD_OUTPUT_ITERATOR_ASSIGN
#  define GECODE_BOOST_HAS_MACRO_USE_FACET
#  ifndef _CPPLIB_VER
      // Updated Dinkum library defines this, and provides
      // its own min and max definitions, as does MTA version.
#     ifndef __MTA__ 
#        define GECODE_BOOST_NO_STD_MIN_MAX
#     endif
#     define GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS
#  endif
#endif

//
// std extension namespace is stdext for vc7.1 and later, 
// the same applies to other compilers that sit on top
// of vc7.1 (Intel and Comeau):
//
#if defined(_MSC_VER) && (_MSC_VER >= 1310) && !defined(__BORLANDC__)
#  define GECODE_BOOST_STD_EXTENSION_NAMESPACE stdext
#endif


#if (defined(_MSC_VER) && (_MSC_VER <= 1300) && !defined(__BORLANDC__)) || !defined(_CPPLIB_VER) || (_CPPLIB_VER < 306)
   // if we're using a dinkum lib that's
   // been configured for VC6/7 then there is
   // no iterator traits (true even for icl)
#  define GECODE_BOOST_NO_STD_ITERATOR_TRAITS
#endif

#if defined(__ICL) && (__ICL < 800) && defined(_CPPLIB_VER) && (_CPPLIB_VER <= 310)
// Intel C++ chokes over any non-trivial use of <locale>
// this may be an overly restrictive define, but regex fails without it:
#  define GECODE_BOOST_NO_STD_LOCALE
#endif

#include <typeinfo>
#if ( (!_HAS_EXCEPTIONS && !defined(__ghs__)) || (!_HAS_NAMESPACE && defined(__ghs__)) ) 
#  define GECODE_BOOST_NO_STD_TYPEINFO    
#endif  

//  C++0x headers implemented in 520 (as shipped by Microsoft)
//
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 520
#  define GECODE_BOOST_NO_0X_HDR_ARRAY
#  define GECODE_BOOST_NO_0X_HDR_CODECVT
#  define GECODE_BOOST_NO_0X_HDR_FORWARD_LIST
#  define GECODE_BOOST_NO_0X_HDR_INITIALIZER_LIST
#  define GECODE_BOOST_NO_0X_HDR_RANDOM
#  define GECODE_BOOST_NO_0X_HDR_REGEX
#  define GECODE_BOOST_NO_0X_HDR_SYSTEM_ERROR
#  define GECODE_BOOST_NO_STD_UNORDERED        // deprecated; see following
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_MAP
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_SET
#  define GECODE_BOOST_NO_0X_HDR_TUPLE
#  define GECODE_BOOST_NO_0X_HDR_TYPEINDEX
#  define GECODE_BOOST_NO_NUMERIC_LIMITS_LOWEST
#endif

#if !defined(_HAS_TR1_IMPORTS) && !defined(GECODE_BOOST_NO_0X_HDR_TUPLE)
#  define GECODE_BOOST_NO_0X_HDR_TUPLE
#endif
//
//  C++0x headers not yet (fully) implemented:
//
#  define GECODE_BOOST_NO_0X_HDR_TYPE_TRAITS
#  define GECODE_BOOST_NO_0X_HDR_CHRONO
#  define GECODE_BOOST_NO_0X_HDR_CONDITION_VARIABLE
#  define GECODE_BOOST_NO_0X_HDR_FUTURE
#  define GECODE_BOOST_NO_0X_HDR_MUTEX
#  define GECODE_BOOST_NO_0X_HDR_RATIO
#  define GECODE_BOOST_NO_0X_HDR_THREAD

#ifdef _CPPLIB_VER
#  define GECODE_BOOST_DINKUMWARE_STDLIB _CPPLIB_VER
#else
#  define GECODE_BOOST_DINKUMWARE_STDLIB 1
#endif

#ifdef _CPPLIB_VER
#  define GECODE_BOOST_STDLIB "Dinkumware standard library version " GECODE_BOOST_STRINGIZE(_CPPLIB_VER)
#else
#  define GECODE_BOOST_STDLIB "Dinkumware standard library version 1.x"
#endif









