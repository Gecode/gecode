//  (C) Copyright John Maddock 2001 - 2003.
//  (C) Copyright David Abrahams 2002 - 2003.
//  (C) Copyright Aleksey Gurtovoy 2002.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  CodeGear C++ compiler setup:

#if !defined( GECODE_BOOST_WITH_CODEGEAR_WARNINGS )
// these warnings occur frequently in optimized template code
# pragma warn -8004 // var assigned value, but never used
# pragma warn -8008 // condition always true/false
# pragma warn -8066 // dead code can never execute
# pragma warn -8104 // static members with ctors not threadsafe
# pragma warn -8105 // reference member in class without ctors
#endif
//
// versions check:
// last known and checked version is 0x621
#if (__CODEGEARC__ > 0x621)
#  if defined(GECODE_BOOST_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  else
#     pragma message( "Unknown compiler version - please run the configure tests and report the results")
#  endif
#endif

// CodeGear C++ Builder 2009
#if (__CODEGEARC__ <= 0x613)
#  define GECODE_BOOST_NO_INTEGRAL_INT64_T
#  define GECODE_BOOST_NO_DEPENDENT_NESTED_DERIVATIONS
#  define GECODE_BOOST_NO_PRIVATE_IN_AGGREGATE
#  define GECODE_BOOST_NO_USING_DECLARATION_OVERLOADS_FROM_TYPENAME_BASE
   // we shouldn't really need this - but too many things choke
   // without it, this needs more investigation:
#  define GECODE_BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
#  define GECODE_BOOST_SP_NO_SP_CONVERTIBLE
#endif

// CodeGear C++ Builder 2010
#if (__CODEGEARC__ <= 0x621)
#  define GECODE_BOOST_NO_TYPENAME_WITH_CTOR    // Cannot use typename keyword when making temporaries of a dependant type
#  define GECODE_BOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL
#  define GECODE_BOOST_NO_MEMBER_TEMPLATE_FRIENDS
#  define GECODE_BOOST_NO_NESTED_FRIENDSHIP     // TC1 gives nested classes access rights as any other member
#  define GECODE_BOOST_NO_USING_TEMPLATE
#  define GECODE_BOOST_NO_TWO_PHASE_NAME_LOOKUP
// Temporary hack, until specific MPL preprocessed headers are generated
#  define GECODE_BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

// CodeGear has not yet completely implemented value-initialization, for
// example for array types, as I reported in 2010: Embarcadero Report 83751,
// "Value-initialization: arrays should have each element value-initialized",
// http://qc.embarcadero.com/wc/qcmain.aspx?d=83751
// Last checked version: Embarcadero C++ 6.21
// See also: http://www.boost.org/libs/utility/value_init.htm#compiler_issues
// (Niels Dekker, LKEB, April 2010)
#  define GECODE_BOOST_NO_COMPLETE_VALUE_INITIALIZATION

#  if defined(NDEBUG) && defined(__cplusplus)
      // fix broken <cstring> so that Boost.test works:
#     include <cstring>
#     undef strcmp
#  endif
   // fix broken errno declaration:
#  include <errno.h>
#  ifndef errno
#     define errno errno
#  endif

#endif
//
// C++0x macros:
//
#if (__CODEGEARC__ <= 0x620)
#define GECODE_BOOST_NO_STATIC_ASSERT
#else
#define GECODE_BOOST_HAS_STATIC_ASSERT
#endif
#define GECODE_BOOST_HAS_CHAR16_T
#define GECODE_BOOST_HAS_CHAR32_T
#define GECODE_BOOST_HAS_LONG_LONG
// #define GECODE_BOOST_HAS_ALIGNOF
#define GECODE_BOOST_HAS_DECLTYPE
#define GECODE_BOOST_HAS_EXPLICIT_CONVERSION_OPS
// #define GECODE_BOOST_HAS_RVALUE_REFS
#define GECODE_BOOST_HAS_SCOPED_ENUM
// #define GECODE_BOOST_HAS_STATIC_ASSERT
#define GECODE_BOOST_HAS_STD_TYPE_TRAITS

#define GECODE_BOOST_NO_AUTO_DECLARATIONS
#define GECODE_BOOST_NO_AUTO_MULTIDECLARATIONS
#define GECODE_BOOST_NO_CONSTEXPR
#define GECODE_BOOST_NO_DEFAULTED_FUNCTIONS
#define GECODE_BOOST_NO_DELETED_FUNCTIONS
#define GECODE_BOOST_NO_EXTERN_TEMPLATE
#define GECODE_BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define GECODE_BOOST_NO_INITIALIZER_LISTS
#define GECODE_BOOST_NO_LAMBDAS
#define GECODE_BOOST_NO_NOEXCEPT
#define GECODE_BOOST_NO_NULLPTR
#define GECODE_BOOST_NO_RAW_LITERALS
#define GECODE_BOOST_NO_RVALUE_REFERENCES
#define GECODE_BOOST_NO_SFINAE_EXPR
#define GECODE_BOOST_NO_TEMPLATE_ALIASES
#define GECODE_BOOST_NO_UNICODE_LITERALS
#define GECODE_BOOST_NO_VARIADIC_TEMPLATES
#define GECODE_BOOST_NO_UNIFIED_INITIALIZATION_SYNTAX

//
// TR1 macros:
//
#define GECODE_BOOST_HAS_TR1_HASH
#define GECODE_BOOST_HAS_TR1_TYPE_TRAITS
#define GECODE_BOOST_HAS_TR1_UNORDERED_MAP
#define GECODE_BOOST_HAS_TR1_UNORDERED_SET

#define GECODE_BOOST_HAS_MACRO_USE_FACET

#define GECODE_BOOST_NO_INITIALIZER_LISTS

// On non-Win32 platforms let the platform config figure this out:
#ifdef _WIN32
#  define GECODE_BOOST_HAS_STDINT_H
#endif

//
// __int64:
//
#if !defined(__STRICT_ANSI__)
#  define GECODE_BOOST_HAS_MS_INT64
#endif
//
// check for exception handling support:
//
#if !defined(_CPPUNWIND) && !defined(GECODE_BOOST_CPPUNWIND) && !defined(__EXCEPTIONS) && !defined(GECODE_BOOST_NO_EXCEPTIONS)
#  define GECODE_BOOST_NO_EXCEPTIONS
#endif
//
// all versions have a <dirent.h>:
//
#if !defined(__STRICT_ANSI__)
#  define GECODE_BOOST_HAS_DIRENT_H
#endif
//
// all versions support __declspec:
//
#if defined(__STRICT_ANSI__)
// config/platform/win32.hpp will define GECODE_BOOST_SYMBOL_EXPORT, etc., unless already defined  
#  define GECODE_BOOST_SYMBOL_EXPORT
#endif
//
// ABI fixing headers:
//
#ifndef GECODE_BOOST_ABI_PREFIX
#  define GECODE_BOOST_ABI_PREFIX "gecode/third-party/boost/config/abi/borland_prefix.hpp"
#endif
#ifndef GECODE_BOOST_ABI_SUFFIX
#  define GECODE_BOOST_ABI_SUFFIX "gecode/third-party/boost/config/abi/borland_suffix.hpp"
#endif
//
// Disable Win32 support in ANSI mode:
//
#  pragma defineonoption GECODE_BOOST_DISABLE_WIN32 -A
//
// MSVC compatibility mode does some nasty things:
// TODO: look up if this doesn't apply to the whole 12xx range
//
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#  define GECODE_BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
#  define GECODE_BOOST_NO_VOID_RETURNS
#endif

#define GECODE_BOOST_COMPILER "CodeGear C++ version " GECODE_BOOST_STRINGIZE(__CODEGEARC__)

