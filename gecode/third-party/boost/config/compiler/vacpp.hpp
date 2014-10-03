//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Toon Knapen 2001 - 2003. 
//  (C) Copyright Lie-Quan Lee 2001. 
//  (C) Copyright Markus Schoepflin 2002 - 2003. 
//  (C) Copyright Beman Dawes 2002 - 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Visual Age (IBM) C++ compiler setup:

#if __IBMCPP__ <= 501
#  define GECODE_BOOST_NO_MEMBER_TEMPLATE_FRIENDS
#  define GECODE_BOOST_NO_MEMBER_FUNCTION_SPECIALIZATIONS
#endif

#if (__IBMCPP__ <= 502) 
// Actually the compiler supports inclass member initialization but it
// requires a definition for the class member and it doesn't recognize
// it as an integral constant expression when used as a template argument.
#  define GECODE_BOOST_NO_INCLASS_MEMBER_INITIALIZATION
#  define GECODE_BOOST_NO_INTEGRAL_INT64_T
#  define GECODE_BOOST_NO_MEMBER_TEMPLATE_KEYWORD
#endif

#if (__IBMCPP__ <= 600) || !defined(GECODE_BOOST_STRICT_CONFIG)
#  define GECODE_BOOST_NO_POINTER_TO_MEMBER_TEMPLATE_PARAMETERS
#endif

#if (__IBMCPP__ <= 1110)
// XL C++ V11.1 and earlier versions may not always value-initialize  
// a temporary object T(), when T is a non-POD aggregate class type. 
// Michael Wong (IBM Canada Ltd) has confirmed this issue and gave it 
// high priority. -- Niels Dekker (LKEB), May 2010.
#  define GECODE_BOOST_NO_COMPLETE_VALUE_INITIALIZATION
#endif

//
// On AIX thread support seems to be indicated by _THREAD_SAFE:
//
#ifdef _THREAD_SAFE
#  define GECODE_BOOST_HAS_THREADS
#endif

#define GECODE_BOOST_COMPILER "IBM Visual Age version " GECODE_BOOST_STRINGIZE(__IBMCPP__)

//
// versions check:
// we don't support Visual age prior to version 5:
#if __IBMCPP__ < 500
#error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 1110:
#if (__IBMCPP__ > 1110)
#  if defined(GECODE_BOOST_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif

// Some versions of the compiler have issues with default arguments on partial specializations
#if __IBMCPP__ <= 1010
#define GECODE_BOOST_NO_PARTIAL_SPECIALIZATION_IMPLICIT_DEFAULT_ARGS
#endif

//
// C++0x features
//
//   See boost\config\suffix.hpp for GECODE_BOOST_NO_LONG_LONG
//
#if ! __IBMCPP_AUTO_TYPEDEDUCTION
#  define GECODE_BOOST_NO_AUTO_DECLARATIONS
#  define GECODE_BOOST_NO_AUTO_MULTIDECLARATIONS
#endif
#if ! __IBMCPP_UTF_LITERAL__
#  define GECODE_BOOST_NO_CHAR16_T
#  define GECODE_BOOST_NO_CHAR32_T
#endif
#define GECODE_BOOST_NO_CONSTEXPR
#if ! __IBMCPP_DECLTYPE
#  define GECODE_BOOST_NO_DECLTYPE
#else
#  define GECODE_BOOST_HAS_DECLTYPE
#endif
#define GECODE_BOOST_NO_DEFAULTED_FUNCTIONS
#define GECODE_BOOST_NO_DELETED_FUNCTIONS
#define GECODE_BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
#if ! __IBMCPP_EXTERN_TEMPLATE
#  define GECODE_BOOST_NO_EXTERN_TEMPLATE
#endif
#if ! __IBMCPP_VARIADIC_TEMPLATES
// not enabled separately at this time
#  define GECODE_BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS
#endif
#define GECODE_BOOST_NO_INITIALIZER_LISTS
#define GECODE_BOOST_NO_LAMBDAS
#define GECODE_BOOST_NO_NOEXCEPT
#define GECODE_BOOST_NO_NULLPTR
#define GECODE_BOOST_NO_RAW_LITERALS
#define GECODE_BOOST_NO_RVALUE_REFERENCES
#define GECODE_BOOST_NO_SCOPED_ENUMS
#define GECODE_BOOST_NO_SFINAE_EXPR
#define GECODE_BOOST_NO_UNIFIED_INITIALIZATION_SYNTAX
#if ! __IBMCPP_STATIC_ASSERT
#  define GECODE_BOOST_NO_STATIC_ASSERT
#endif
#define GECODE_BOOST_NO_TEMPLATE_ALIASES
#define GECODE_BOOST_NO_UNICODE_LITERALS
#if ! __IBMCPP_VARIADIC_TEMPLATES
#  define GECODE_BOOST_NO_VARIADIC_TEMPLATES
#endif
#if ! __C99_MACRO_WITH_VA_ARGS
#  define GECODE_BOOST_NO_VARIADIC_MACROS
#endif



