//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Jens Maurer 2001. 
//  (C) Copyright David Abrahams 2003. 
//  (C) Copyright Boris Gubenko 2007. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Rogue Wave std lib:

#define GECODE_BOOST_RW_STDLIB 1 

#if !defined(__STD_RWCOMPILER_H__) && !defined(_RWSTD_VER)
#  include <gecode/third-party/boost/config/no_tr1/utility.hpp>
#  if !defined(__STD_RWCOMPILER_H__) && !defined(_RWSTD_VER)
#     error This is not the Rogue Wave standard library
#  endif
#endif
//
// figure out a consistent version number:
//
#ifndef _RWSTD_VER
#  define GECODE_BOOST_RWSTD_VER 0x010000
#elif _RWSTD_VER < 0x010000
#  define GECODE_BOOST_RWSTD_VER (_RWSTD_VER << 8)
#else
#  define GECODE_BOOST_RWSTD_VER _RWSTD_VER
#endif

#ifndef _RWSTD_VER
#  define GECODE_BOOST_STDLIB "Rogue Wave standard library version (Unknown version)"
#elif _RWSTD_VER < 0x04010200
 #  define GECODE_BOOST_STDLIB "Rogue Wave standard library version " GECODE_BOOST_STRINGIZE(_RWSTD_VER)
#else
#  ifdef _RWSTD_VER_STR
#    define GECODE_BOOST_STDLIB "Apache STDCXX standard library version " _RWSTD_VER_STR
#  else
#    define GECODE_BOOST_STDLIB "Apache STDCXX standard library version " GECODE_BOOST_STRINGIZE(_RWSTD_VER)
#  endif
#endif

//
// Prior to version 2.2.0 the primary template for std::numeric_limits
// does not have compile time constants, even though specializations of that
// template do:
//
#if GECODE_BOOST_RWSTD_VER < 0x020200
#  define GECODE_BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
#endif

// Sun CC 5.5 patch 113817-07 adds long long specialization, but does not change the
// library version number (http://sunsolve6.sun.com/search/document.do?assetkey=1-21-113817):
#if GECODE_BOOST_RWSTD_VER <= 0x020101 && (!defined(__SUNPRO_CC) || (__SUNPRO_CC < 0x550))
#  define GECODE_BOOST_NO_LONG_LONG_NUMERIC_LIMITS
# endif

//
// Borland version of numeric_limits lacks __int64 specialisation:
//
#ifdef __BORLANDC__
#  define GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS
#endif

//
// No std::iterator if it can't figure out default template args:
//
#if defined(_RWSTD_NO_SIMPLE_DEFAULT_TEMPLATES) || defined(RWSTD_NO_SIMPLE_DEFAULT_TEMPLATES) || (GECODE_BOOST_RWSTD_VER < 0x020000)
#  define GECODE_BOOST_NO_STD_ITERATOR
#endif

//
// No iterator traits without partial specialization:
//
#if defined(_RWSTD_NO_CLASS_PARTIAL_SPEC) || defined(RWSTD_NO_CLASS_PARTIAL_SPEC)
#  define GECODE_BOOST_NO_STD_ITERATOR_TRAITS
#endif

//
// Prior to version 2.0, std::auto_ptr was buggy, and there were no
// new-style iostreams, and no conformant std::allocator:
//
#if (GECODE_BOOST_RWSTD_VER < 0x020000)
#  define GECODE_BOOST_NO_AUTO_PTR
#  define GECODE_BOOST_NO_STRINGSTREAM
#  define GECODE_BOOST_NO_STD_ALLOCATOR
#  define GECODE_BOOST_NO_STD_LOCALE
#endif

//
// No template iterator constructors without member template support:
//
#if defined(RWSTD_NO_MEMBER_TEMPLATES) || defined(_RWSTD_NO_MEMBER_TEMPLATES)
#  define GECODE_BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
#endif

//
// RW defines _RWSTD_ALLOCATOR if the allocator is conformant and in use
// (the or _HPACC_ part is a hack - the library seems to define _RWSTD_ALLOCATOR
// on HP aCC systems even though the allocator is in fact broken):
//
#if !defined(_RWSTD_ALLOCATOR) || (defined(__HP_aCC) && __HP_aCC <= 33100)
#  define GECODE_BOOST_NO_STD_ALLOCATOR
#endif

//
// If we have a std::locale, we still may not have std::use_facet:
//
#if defined(_RWSTD_NO_TEMPLATE_ON_RETURN_TYPE) && !defined(GECODE_BOOST_NO_STD_LOCALE)
#  define GECODE_BOOST_NO_STD_USE_FACET
#  define GECODE_BOOST_HAS_TWO_ARG_USE_FACET
#endif

//
// There's no std::distance prior to version 2, or without
// partial specialization support:
//
#if (GECODE_BOOST_RWSTD_VER < 0x020000) || defined(_RWSTD_NO_CLASS_PARTIAL_SPEC)
    #define GECODE_BOOST_NO_STD_DISTANCE
#endif

//
// Some versions of the rogue wave library don't have assignable
// OutputIterators:
//
#if GECODE_BOOST_RWSTD_VER < 0x020100
#  define GECODE_BOOST_NO_STD_OUTPUT_ITERATOR_ASSIGN
#endif

//
// Disable GECODE_BOOST_HAS_LONG_LONG when the library has no support for it.
//
#if !defined(_RWSTD_LONG_LONG) && defined(GECODE_BOOST_HAS_LONG_LONG)
#  undef GECODE_BOOST_HAS_LONG_LONG
#endif

//
// check that on HP-UX, the proper RW library is used
//
#if defined(__HP_aCC) && !defined(_HP_NAMESPACE_STD)
#  error "Boost requires Standard RW library. Please compile and link with -AA"
#endif

//
// Define macros specific to RW V2.2 on HP-UX
//
#if defined(__HP_aCC) && (GECODE_BOOST_RWSTD_VER == 0x02020100)
#  ifndef __HP_TC1_MAKE_PAIR
#    define __HP_TC1_MAKE_PAIR
#  endif
#  ifndef _HP_INSTANTIATE_STD2_VL
#    define _HP_INSTANTIATE_STD2_VL
#  endif
#endif

#if _RWSTD_VER < 0x05000000
#  define GECODE_BOOST_NO_0X_HDR_ARRAY
#endif
// type_traits header is incomplete:
#  define GECODE_BOOST_NO_0X_HDR_TYPE_TRAITS
//
//  C++0x headers not yet implemented
//
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
#  define GECODE_BOOST_NO_0X_HDR_TYPEINDEX
#  define GECODE_BOOST_NO_STD_UNORDERED        // deprecated; see following
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_MAP
#  define GECODE_BOOST_NO_0X_HDR_UNORDERED_SET
#  define GECODE_BOOST_NO_NUMERIC_LIMITS_LOWEST

