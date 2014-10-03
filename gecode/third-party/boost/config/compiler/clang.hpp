// (C) Copyright Douglas Gregor 2010
//
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

// Clang compiler setup.

#if __has_feature(cxx_exceptions) && !defined(GECODE_BOOST_NO_EXCEPTIONS)
#else
#  define GECODE_BOOST_NO_EXCEPTIONS
#endif

#if !__has_feature(cxx_rtti)
#  define GECODE_BOOST_NO_RTTI
#endif

#if defined(__int64)
#  define GECODE_BOOST_HAS_MS_INT64
#endif

#define GECODE_BOOST_HAS_NRVO

// Clang supports "long long" in all compilation modes.

#define GECODE_BOOST_NO_AUTO_DECLARATIONS
#define GECODE_BOOST_NO_AUTO_MULTIDECLARATIONS
#define GECODE_BOOST_NO_CHAR16_T
#define GECODE_BOOST_NO_CHAR32_T
#define GECODE_BOOST_NO_CONSTEXPR

#if !__has_feature(cxx_decltype)
#  define GECODE_BOOST_NO_DECLTYPE
#endif

#define GECODE_BOOST_NO_DEFAULTED_FUNCTIONS

#if !__has_feature(cxx_deleted_functions)
#  define GECODE_BOOST_NO_DELETED_FUNCTIONS
#endif

#define GECODE_BOOST_NO_EXPLICIT_CONVERSION_OPERATORS

#if !__has_feature(cxx_default_function_template_args)
  #define GECODE_BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS
#endif

#define GECODE_BOOST_NO_INITIALIZER_LISTS
#define GECODE_BOOST_NO_LAMBDAS
#define GECODE_BOOST_NO_NOEXCEPT
#define GECODE_BOOST_NO_NULLPTR
#define GECODE_BOOST_NO_RAW_LITERALS
#define GECODE_BOOST_NO_UNIFIED_INITIALIZATION_SYNTAX

#if !__has_feature(cxx_rvalue_references)
#  define GECODE_BOOST_NO_RVALUE_REFERENCES
#endif

#if !__has_feature(cxx_strong_enums)
#  define GECODE_BOOST_NO_SCOPED_ENUMS
#endif

#if !__has_feature(cxx_static_assert)
#  define GECODE_BOOST_NO_STATIC_ASSERT
#endif

#define GECODE_BOOST_NO_TEMPLATE_ALIASES
#define GECODE_BOOST_NO_UNICODE_LITERALS

#if !__has_feature(cxx_variadic_templates)
#  define GECODE_BOOST_NO_VARIADIC_TEMPLATES
#endif

// Clang always supports variadic macros
// Clang always supports extern templates

#ifndef GECODE_BOOST_COMPILER
#  define GECODE_BOOST_COMPILER "Clang version " __clang_version__
#endif

// Macro used to identify the Clang compiler.
#define GECODE_BOOST_CLANG 1

