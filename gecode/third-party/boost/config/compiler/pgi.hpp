//  (C) Copyright Noel Belcourt 2007.
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  PGI C++ compiler setup:

#define GECODE_BOOST_COMPILER_VERSION __PGIC__##__PGIC_MINOR__
#define GECODE_BOOST_COMPILER "PGI compiler version " GECODE_BOOST_STRINGIZE(_COMPILER_VERSION)

//
// Threading support:
// Turn this on unconditionally here, it will get turned off again later
// if no threading API is detected.
//

// PGI 10.x doesn't seem to define __PGIC__

// versions earlier than 10.x do define __PGIC__
#if __PGIC__ >= 10

// options requested by configure --enable-test
#define GECODE_BOOST_HAS_PTHREADS
#define GECODE_BOOST_HAS_NRVO
#define GECODE_BOOST_HAS_LONG_LONG

// options --enable-test wants undefined
#undef GECODE_BOOST_NO_STDC_NAMESPACE
#undef GECODE_BOOST_NO_EXCEPTION_STD_NAMESPACE
#undef GECODE_BOOST_DEDUCED_TYPENAME

#elif __PGIC__ >= 7

#define GECODE_BOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL 
#define GECODE_BOOST_NO_TWO_PHASE_NAME_LOOKUP
#define GECODE_BOOST_NO_SWPRINTF
#define GECODE_BOOST_NO_AUTO_MULTIDECLARATIONS
#define GECODE_BOOST_NO_AUTO_DECLARATIONS

#else

#  error "Pgi compiler not configured - please reconfigure"

#endif
//
// C++0x features
//
//   See boost\config\suffix.hpp for GECODE_BOOST_NO_LONG_LONG
//
#define GECODE_BOOST_NO_CHAR16_T
#define GECODE_BOOST_NO_CHAR32_T
#define GECODE_BOOST_NO_CONSTEXPR
#define GECODE_BOOST_NO_DECLTYPE
#define GECODE_BOOST_NO_DEFAULTED_FUNCTIONS
#define GECODE_BOOST_NO_DELETED_FUNCTIONS
#define GECODE_BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
#define GECODE_BOOST_NO_EXTERN_TEMPLATE
#define GECODE_BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define GECODE_BOOST_NO_INITIALIZER_LISTS
#define GECODE_BOOST_NO_LAMBDAS
#define GECODE_BOOST_NO_NOEXCEPT
#define GECODE_BOOST_NO_NULLPTR
#define GECODE_BOOST_NO_RAW_LITERALS
#define GECODE_BOOST_NO_RVALUE_REFERENCES
#define GECODE_BOOST_NO_SCOPED_ENUMS
#define GECODE_BOOST_NO_SFINAE_EXPR
#define GECODE_BOOST_NO_STATIC_ASSERT
#define GECODE_BOOST_NO_TEMPLATE_ALIASES
#define GECODE_BOOST_NO_UNICODE_LITERALS
#define GECODE_BOOST_NO_VARIADIC_TEMPLATES
#define GECODE_BOOST_NO_VARIADIC_MACROS
#define GECODE_BOOST_NO_UNIFIED_INITIALIZATION_SYNTAX

//
// version check:
// probably nothing to do here?

