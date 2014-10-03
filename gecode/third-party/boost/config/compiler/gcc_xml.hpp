//  (C) Copyright John Maddock 2006. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  GCC-XML C++ compiler setup:

#  if !defined(__GCCXML_GNUC__) || ((__GCCXML_GNUC__ <= 3) && (__GCCXML_GNUC_MINOR__ <= 3))
#     define GECODE_BOOST_NO_IS_ABSTRACT
#  endif

//
// Threading support: Turn this on unconditionally here (except for
// those platforms where we can know for sure). It will get turned off again
// later if no threading API is detected.
//
#if !defined(__MINGW32__) && !defined(_MSC_VER) && !defined(linux) && !defined(__linux) && !defined(__linux__)
# define GECODE_BOOST_HAS_THREADS
#endif 

//
// gcc has "long long"
//
#define GECODE_BOOST_HAS_LONG_LONG

// C++0x features:
//
#  define GECODE_BOOST_NO_CONSTEXPR
#  define GECODE_BOOST_NO_NULLPTR
#  define GECODE_BOOST_NO_TEMPLATE_ALIASES
#  define GECODE_BOOST_NO_DECLTYPE
#  define GECODE_BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS
#  define GECODE_BOOST_NO_RVALUE_REFERENCES
#  define GECODE_BOOST_NO_STATIC_ASSERT
#  define GECODE_BOOST_NO_VARIADIC_TEMPLATES
#  define GECODE_BOOST_NO_VARIADIC_MACROS
#  define GECODE_BOOST_NO_AUTO_DECLARATIONS
#  define GECODE_BOOST_NO_AUTO_MULTIDECLARATIONS
#  define GECODE_BOOST_NO_CHAR16_T
#  define GECODE_BOOST_NO_CHAR32_T
#  define GECODE_BOOST_NO_DEFAULTED_FUNCTIONS
#  define GECODE_BOOST_NO_DELETED_FUNCTIONS
#  define GECODE_BOOST_NO_INITIALIZER_LISTS
#  define GECODE_BOOST_NO_SCOPED_ENUMS  
#  define GECODE_BOOST_NO_SFINAE_EXPR
#  define GECODE_BOOST_NO_SCOPED_ENUMS
#  define GECODE_BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
#  define GECODE_BOOST_NO_LAMBDAS
#  define GECODE_BOOST_NO_RAW_LITERALS
#  define GECODE_BOOST_NO_UNICODE_LITERALS
#  define GECODE_BOOST_NO_NOEXCEPT
#define GECODE_BOOST_NO_UNIFIED_INITIALIZATION_SYNTAX

#define GECODE_BOOST_COMPILER "GCC-XML C++ version " __GCCXML__


