//  Boost config.hpp configuration header file  ------------------------------//

//  Copyright (c) 2001-2003 John Maddock
//  Copyright (c) 2001 Darin Adler
//  Copyright (c) 2001 Peter Dimov
//  Copyright (c) 2002 Bill Kempf
//  Copyright (c) 2002 Jens Maurer
//  Copyright (c) 2002-2003 David Abrahams
//  Copyright (c) 2003 Gennaro Prota
//  Copyright (c) 2003 Eric Friedman
//  Copyright (c) 2010 Eric Jourdanneau, Joel Falcou
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/ for most recent version.

//  Boost config.hpp policy and rationale documentation has been moved to
//  http://www.boost.org/libs/config/
//
//  This file is intended to be stable, and relatively unchanging.
//  It should contain boilerplate code only - no compiler specific
//  code unless it is unavoidable - no changes unless unavoidable.

#ifndef GECODE_BOOST_CONFIG_SUFFIX_HPP
#define GECODE_BOOST_CONFIG_SUFFIX_HPP

#if defined(__GNUC__) && (__GNUC__ >= 4)
//
// Some GCC-4.x versions issue warnings even when __extension__ is used,
// so use this as a workaround:
//
#pragma GCC system_header
#endif

//
// ensure that visibility macros are always defined, thus symplifying use
//
#ifndef GECODE_BOOST_SYMBOL_EXPORT
# define GECODE_BOOST_SYMBOL_EXPORT
#endif
#ifndef GECODE_BOOST_SYMBOL_IMPORT
# define GECODE_BOOST_SYMBOL_IMPORT
#endif
#ifndef GECODE_BOOST_SYMBOL_VISIBLE
# define GECODE_BOOST_SYMBOL_VISIBLE
#endif

//
// look for long long by looking for the appropriate macros in <limits.h>.
// Note that we use limits.h rather than climits for maximal portability,
// remember that since these just declare a bunch of macros, there should be
// no namespace issues from this.
//
#if !defined(GECODE_BOOST_HAS_LONG_LONG) && !defined(GECODE_BOOST_NO_LONG_LONG)                                              \
   && !defined(GECODE_BOOST_MSVC) && !defined(__BORLANDC__)
# include <limits.h>
# if (defined(ULLONG_MAX) || defined(ULONG_LONG_MAX) || defined(ULONGLONG_MAX))
#   define GECODE_BOOST_HAS_LONG_LONG
# else
#   define GECODE_BOOST_NO_LONG_LONG
# endif
#endif

// GCC 3.x will clean up all of those nasty macro definitions that
// GECODE_BOOST_NO_CTYPE_FUNCTIONS is intended to help work around, so undefine
// it under GCC 3.x.
#if defined(__GNUC__) && (__GNUC__ >= 3) && defined(GECODE_BOOST_NO_CTYPE_FUNCTIONS)
#  undef GECODE_BOOST_NO_CTYPE_FUNCTIONS
#endif

//
// Assume any extensions are in namespace std:: unless stated otherwise:
//
#  ifndef GECODE_BOOST_STD_EXTENSION_NAMESPACE
#    define GECODE_BOOST_STD_EXTENSION_NAMESPACE std
#  endif

//
// If cv-qualified specializations are not allowed, then neither are cv-void ones:
//
#  if defined(GECODE_BOOST_NO_CV_SPECIALIZATIONS) \
      && !defined(GECODE_BOOST_NO_CV_VOID_SPECIALIZATIONS)
#     define GECODE_BOOST_NO_CV_VOID_SPECIALIZATIONS
#  endif

//
// If there is no numeric_limits template, then it can't have any compile time
// constants either!
//
#  if defined(GECODE_BOOST_NO_LIMITS) \
      && !defined(GECODE_BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS)
#     define GECODE_BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
#     define GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS
#     define GECODE_BOOST_NO_LONG_LONG_NUMERIC_LIMITS
#  endif

//
// if there is no long long then there is no specialisation
// for numeric_limits<long long> either:
//
#if !defined(GECODE_BOOST_HAS_LONG_LONG) && !defined(GECODE_BOOST_NO_LONG_LONG_NUMERIC_LIMITS)
#  define GECODE_BOOST_NO_LONG_LONG_NUMERIC_LIMITS
#endif

//
// Normalize GECODE_BOOST_NO_STATIC_ASSERT and (depricated) GECODE_BOOST_HAS_STATIC_ASSERT:
//
#if !defined(GECODE_BOOST_NO_STATIC_ASSERT) && !defined(GECODE_BOOST_HAS_STATIC_ASSERT)
#  define GECODE_BOOST_HAS_STATIC_ASSERT
#endif

//
// if there is no __int64 then there is no specialisation
// for numeric_limits<__int64> either:
//
#if !defined(GECODE_BOOST_HAS_MS_INT64) && !defined(GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS)
#  define GECODE_BOOST_NO_MS_INT64_NUMERIC_LIMITS
#endif

//
// if member templates are supported then so is the
// VC6 subset of member templates:
//
#  if !defined(GECODE_BOOST_NO_MEMBER_TEMPLATES) \
       && !defined(GECODE_BOOST_MSVC6_MEMBER_TEMPLATES)
#     define GECODE_BOOST_MSVC6_MEMBER_TEMPLATES
#  endif

//
// Without partial specialization, can't test for partial specialisation bugs:
//
#  if defined(GECODE_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
      && !defined(GECODE_BOOST_BCB_PARTIAL_SPECIALIZATION_BUG)
#     define GECODE_BOOST_BCB_PARTIAL_SPECIALIZATION_BUG
#  endif

//
// Without partial specialization, we can't have array-type partial specialisations:
//
#  if defined(GECODE_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
      && !defined(GECODE_BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS)
#     define GECODE_BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS
#  endif

//
// Without partial specialization, std::iterator_traits can't work:
//
#  if defined(GECODE_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
      && !defined(GECODE_BOOST_NO_STD_ITERATOR_TRAITS)
#     define GECODE_BOOST_NO_STD_ITERATOR_TRAITS
#  endif

//
// Without partial specialization, partial
// specialization with default args won't work either:
//
#  if defined(GECODE_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
      && !defined(GECODE_BOOST_NO_PARTIAL_SPECIALIZATION_IMPLICIT_DEFAULT_ARGS)
#     define GECODE_BOOST_NO_PARTIAL_SPECIALIZATION_IMPLICIT_DEFAULT_ARGS
#  endif

//
// Without member template support, we can't have template constructors
// in the standard library either:
//
#  if defined(GECODE_BOOST_NO_MEMBER_TEMPLATES) \
      && !defined(GECODE_BOOST_MSVC6_MEMBER_TEMPLATES) \
      && !defined(GECODE_BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS)
#     define GECODE_BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
#  endif

//
// Without member template support, we can't have a conforming
// std::allocator template either:
//
#  if defined(GECODE_BOOST_NO_MEMBER_TEMPLATES) \
      && !defined(GECODE_BOOST_MSVC6_MEMBER_TEMPLATES) \
      && !defined(GECODE_BOOST_NO_STD_ALLOCATOR)
#     define GECODE_BOOST_NO_STD_ALLOCATOR
#  endif

//
// without ADL support then using declarations will break ADL as well:
//
#if defined(GECODE_BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP) && !defined(GECODE_BOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL)
#  define GECODE_BOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL
#endif

//
// Without typeid support we have no dynamic RTTI either:
//
#if defined(GECODE_BOOST_NO_TYPEID) && !defined(GECODE_BOOST_NO_RTTI)
#  define GECODE_BOOST_NO_RTTI
#endif

//
// If we have a standard allocator, then we have a partial one as well:
//
#if !defined(GECODE_BOOST_NO_STD_ALLOCATOR)
#  define GECODE_BOOST_HAS_PARTIAL_STD_ALLOCATOR
#endif

//
// We can't have a working std::use_facet if there is no std::locale:
//
#  if defined(GECODE_BOOST_NO_STD_LOCALE) && !defined(GECODE_BOOST_NO_STD_USE_FACET)
#     define GECODE_BOOST_NO_STD_USE_FACET
#  endif

//
// We can't have a std::messages facet if there is no std::locale:
//
#  if defined(GECODE_BOOST_NO_STD_LOCALE) && !defined(GECODE_BOOST_NO_STD_MESSAGES)
#     define GECODE_BOOST_NO_STD_MESSAGES
#  endif

//
// We can't have a working std::wstreambuf if there is no std::locale:
//
#  if defined(GECODE_BOOST_NO_STD_LOCALE) && !defined(GECODE_BOOST_NO_STD_WSTREAMBUF)
#     define GECODE_BOOST_NO_STD_WSTREAMBUF
#  endif

//
// We can't have a <cwctype> if there is no <cwchar>:
//
#  if defined(GECODE_BOOST_NO_CWCHAR) && !defined(GECODE_BOOST_NO_CWCTYPE)
#     define GECODE_BOOST_NO_CWCTYPE
#  endif

//
// We can't have a swprintf if there is no <cwchar>:
//
#  if defined(GECODE_BOOST_NO_CWCHAR) && !defined(GECODE_BOOST_NO_SWPRINTF)
#     define GECODE_BOOST_NO_SWPRINTF
#  endif

//
// If Win32 support is turned off, then we must turn off
// threading support also, unless there is some other
// thread API enabled:
//
#if defined(GECODE_BOOST_DISABLE_WIN32) && defined(_WIN32) \
   && !defined(GECODE_BOOST_DISABLE_THREADS) && !defined(GECODE_BOOST_HAS_PTHREADS)
#  define GECODE_BOOST_DISABLE_THREADS
#endif

//
// Turn on threading support if the compiler thinks that it's in
// multithreaded mode.  We put this here because there are only a
// limited number of macros that identify this (if there's any missing
// from here then add to the appropriate compiler section):
//
#if (defined(__MT__) || defined(_MT) || defined(_REENTRANT) \
    || defined(_PTHREADS) || defined(__APPLE__) || defined(__DragonFly__)) \
    && !defined(GECODE_BOOST_HAS_THREADS)
#  define GECODE_BOOST_HAS_THREADS
#endif

//
// Turn threading support off if GECODE_BOOST_DISABLE_THREADS is defined:
//
#if defined(GECODE_BOOST_DISABLE_THREADS) && defined(GECODE_BOOST_HAS_THREADS)
#  undef GECODE_BOOST_HAS_THREADS
#endif

//
// Turn threading support off if we don't recognise the threading API:
//
#if defined(GECODE_BOOST_HAS_THREADS) && !defined(GECODE_BOOST_HAS_PTHREADS)\
      && !defined(GECODE_BOOST_HAS_WINTHREADS) && !defined(GECODE_BOOST_HAS_BETHREADS)\
      && !defined(GECODE_BOOST_HAS_MPTASKS)
#  undef GECODE_BOOST_HAS_THREADS
#endif

//
// Turn threading detail macros off if we don't (want to) use threading
//
#ifndef GECODE_BOOST_HAS_THREADS
#  undef GECODE_BOOST_HAS_PTHREADS
#  undef GECODE_BOOST_HAS_PTHREAD_MUTEXATTR_SETTYPE
#  undef GECODE_BOOST_HAS_PTHREAD_YIELD
#  undef GECODE_BOOST_HAS_PTHREAD_DELAY_NP
#  undef GECODE_BOOST_HAS_WINTHREADS
#  undef GECODE_BOOST_HAS_BETHREADS
#  undef GECODE_BOOST_HAS_MPTASKS
#endif

//
// If the compiler claims to be C99 conformant, then it had better
// have a <stdint.h>:
//
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#     define GECODE_BOOST_HAS_STDINT_H
#     ifndef GECODE_BOOST_HAS_LOG1P
#        define GECODE_BOOST_HAS_LOG1P
#     endif
#     ifndef GECODE_BOOST_HAS_EXPM1
#        define GECODE_BOOST_HAS_EXPM1
#     endif
#  endif

//
// Define GECODE_BOOST_NO_SLIST and GECODE_BOOST_NO_HASH if required.
// Note that this is for backwards compatibility only.
//
#  if !defined(GECODE_BOOST_HAS_SLIST) && !defined(GECODE_BOOST_NO_SLIST)
#     define GECODE_BOOST_NO_SLIST
#  endif

#  if !defined(GECODE_BOOST_HAS_HASH) && !defined(GECODE_BOOST_NO_HASH)
#     define GECODE_BOOST_NO_HASH
#  endif

//
// Set GECODE_BOOST_SLIST_HEADER if not set already:
//
#if defined(GECODE_BOOST_HAS_SLIST) && !defined(GECODE_BOOST_SLIST_HEADER)
#  define GECODE_BOOST_SLIST_HEADER <slist>
#endif

//
// Set GECODE_BOOST_HASH_SET_HEADER if not set already:
//
#if defined(GECODE_BOOST_HAS_HASH) && !defined(GECODE_BOOST_HASH_SET_HEADER)
#  define GECODE_BOOST_HASH_SET_HEADER <hash_set>
#endif

//
// Set GECODE_BOOST_HASH_MAP_HEADER if not set already:
//
#if defined(GECODE_BOOST_HAS_HASH) && !defined(GECODE_BOOST_HASH_MAP_HEADER)
#  define GECODE_BOOST_HASH_MAP_HEADER <hash_map>
#endif

//
// Set GECODE_BOOST_NO_INITIALIZER_LISTS if there is no library support.
//

#if defined(GECODE_BOOST_NO_0X_HDR_INITIALIZER_LIST) && !defined(GECODE_BOOST_NO_INITIALIZER_LISTS)
#  define GECODE_BOOST_NO_INITIALIZER_LISTS
#endif
#if defined(GECODE_BOOST_NO_INITIALIZER_LISTS) && !defined(GECODE_BOOST_NO_0X_HDR_INITIALIZER_LIST)
#  define GECODE_BOOST_NO_0X_HDR_INITIALIZER_LIST
#endif

//
// Set GECODE_BOOST_HAS_RVALUE_REFS when GECODE_BOOST_NO_RVALUE_REFERENCES is not defined
//
#if !defined(GECODE_BOOST_NO_RVALUE_REFERENCES) && !defined(GECODE_BOOST_HAS_RVALUE_REFS)
#define GECODE_BOOST_HAS_RVALUE_REFS
#endif

//
// Set GECODE_BOOST_HAS_VARIADIC_TMPL when GECODE_BOOST_NO_VARIADIC_TEMPLATES is not defined
//
#if !defined(GECODE_BOOST_NO_VARIADIC_TEMPLATES) && !defined(GECODE_BOOST_HAS_VARIADIC_TMPL)
#define GECODE_BOOST_HAS_VARIADIC_TMPL
#endif

//  GECODE_BOOST_HAS_ABI_HEADERS
//  This macro gets set if we have headers that fix the ABI,
//  and prevent ODR violations when linking to external libraries:
#if defined(GECODE_BOOST_ABI_PREFIX) && defined(GECODE_BOOST_ABI_SUFFIX) && !defined(GECODE_BOOST_HAS_ABI_HEADERS)
#  define GECODE_BOOST_HAS_ABI_HEADERS
#endif

#if defined(GECODE_BOOST_HAS_ABI_HEADERS) && defined(GECODE_BOOST_DISABLE_ABI_HEADERS)
#  undef GECODE_BOOST_HAS_ABI_HEADERS
#endif

//  GECODE_BOOST_NO_STDC_NAMESPACE workaround  --------------------------------------//
//  Because std::size_t usage is so common, even in boost headers which do not
//  otherwise use the C library, the <cstddef> workaround is included here so
//  that ugly workaround code need not appear in many other boost headers.
//  NOTE WELL: This is a workaround for non-conforming compilers; <cstddef>
//  must still be #included in the usual places so that <cstddef> inclusion
//  works as expected with standard conforming compilers.  The resulting
//  double inclusion of <cstddef> is harmless.

# if defined(GECODE_BOOST_NO_STDC_NAMESPACE) && defined(__cplusplus)
#   include <cstddef>
    namespace std { using ::ptrdiff_t; using ::size_t; }
# endif

//  Workaround for the unfortunate min/max macros defined by some platform headers

#define GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION

#ifndef GECODE_BOOST_USING_STD_MIN
#  define GECODE_BOOST_USING_STD_MIN() using std::min
#endif

#ifndef GECODE_BOOST_USING_STD_MAX
#  define GECODE_BOOST_USING_STD_MAX() using std::max
#endif

//  GECODE_BOOST_NO_STD_MIN_MAX workaround  -----------------------------------------//

#  if defined(GECODE_BOOST_NO_STD_MIN_MAX) && defined(__cplusplus)

namespace std {
  template <class _Tp>
  inline const _Tp& min GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (const _Tp& __a, const _Tp& __b) {
    return __b < __a ? __b : __a;
  }
  template <class _Tp>
  inline const _Tp& max GECODE_BOOST_PREVENT_MACRO_SUBSTITUTION (const _Tp& __a, const _Tp& __b) {
    return  __a < __b ? __b : __a;
  }
}

#  endif

// GECODE_BOOST_STATIC_CONSTANT workaround --------------------------------------- //
// On compilers which don't allow in-class initialization of static integral
// constant members, we must use enums as a workaround if we want the constants
// to be available at compile-time. This macro gives us a convenient way to
// declare such constants.

#  ifdef GECODE_BOOST_NO_INCLASS_MEMBER_INITIALIZATION
#       define GECODE_BOOST_STATIC_CONSTANT(type, assignment) enum { assignment }
#  else
#     define GECODE_BOOST_STATIC_CONSTANT(type, assignment) static const type assignment
#  endif

// GECODE_BOOST_USE_FACET / HAS_FACET workaround ----------------------------------//
// When the standard library does not have a conforming std::use_facet there
// are various workarounds available, but they differ from library to library.
// The same problem occurs with has_facet.
// These macros provide a consistent way to access a locale's facets.
// Usage:
//    replace
//       std::use_facet<Type>(loc);
//    with
//       GECODE_BOOST_USE_FACET(Type, loc);
//    Note do not add a std:: prefix to the front of GECODE_BOOST_USE_FACET!
//  Use for GECODE_BOOST_HAS_FACET is analogous.

#if defined(GECODE_BOOST_NO_STD_USE_FACET)
#  ifdef GECODE_BOOST_HAS_TWO_ARG_USE_FACET
#     define GECODE_BOOST_USE_FACET(Type, loc) std::use_facet(loc, static_cast<Type*>(0))
#     define GECODE_BOOST_HAS_FACET(Type, loc) std::has_facet(loc, static_cast<Type*>(0))
#  elif defined(GECODE_BOOST_HAS_MACRO_USE_FACET)
#     define GECODE_BOOST_USE_FACET(Type, loc) std::_USE(loc, Type)
#     define GECODE_BOOST_HAS_FACET(Type, loc) std::_HAS(loc, Type)
#  elif defined(GECODE_BOOST_HAS_STLP_USE_FACET)
#     define GECODE_BOOST_USE_FACET(Type, loc) (*std::_Use_facet<Type >(loc))
#     define GECODE_BOOST_HAS_FACET(Type, loc) std::has_facet< Type >(loc)
#  endif
#else
#  define GECODE_BOOST_USE_FACET(Type, loc) std::use_facet< Type >(loc)
#  define GECODE_BOOST_HAS_FACET(Type, loc) std::has_facet< Type >(loc)
#endif

// GECODE_BOOST_NESTED_TEMPLATE workaround ------------------------------------------//
// Member templates are supported by some compilers even though they can't use
// the A::template member<U> syntax, as a workaround replace:
//
// typedef typename A::template rebind<U> binder;
//
// with:
//
// typedef typename A::GECODE_BOOST_NESTED_TEMPLATE rebind<U> binder;

#ifndef GECODE_BOOST_NO_MEMBER_TEMPLATE_KEYWORD
#  define GECODE_BOOST_NESTED_TEMPLATE template
#else
#  define GECODE_BOOST_NESTED_TEMPLATE
#endif

// GECODE_BOOST_UNREACHABLE_RETURN(x) workaround -------------------------------------//
// Normally evaluates to nothing, unless GECODE_BOOST_NO_UNREACHABLE_RETURN_DETECTION
// is defined, in which case it evaluates to return x; Use when you have a return
// statement that can never be reached.

#ifdef GECODE_BOOST_NO_UNREACHABLE_RETURN_DETECTION
#  define GECODE_BOOST_UNREACHABLE_RETURN(x) return x;
#else
#  define GECODE_BOOST_UNREACHABLE_RETURN(x)
#endif

// GECODE_BOOST_DEDUCED_TYPENAME workaround ------------------------------------------//
//
// Some compilers don't support the use of `typename' for dependent
// types in deduced contexts, e.g.
//
//     template <class T> void f(T, typename T::type);
//                                  ^^^^^^^^
// Replace these declarations with:
//
//     template <class T> void f(T, GECODE_BOOST_DEDUCED_TYPENAME T::type);

#ifndef GECODE_BOOST_NO_DEDUCED_TYPENAME
#  define GECODE_BOOST_DEDUCED_TYPENAME typename
#else
#  define GECODE_BOOST_DEDUCED_TYPENAME
#endif

#ifndef GECODE_BOOST_NO_TYPENAME_WITH_CTOR
#  define GECODE_BOOST_CTOR_TYPENAME typename
#else
#  define GECODE_BOOST_CTOR_TYPENAME
#endif

// long long workaround ------------------------------------------//
// On gcc (and maybe other compilers?) long long is alway supported
// but it's use may generate either warnings (with -ansi), or errors
// (with -pedantic -ansi) unless it's use is prefixed by __extension__
//
#if defined(GECODE_BOOST_HAS_LONG_LONG) && defined(__cplusplus)
namespace gecode_boost{
#  ifdef __GNUC__
   __extension__ typedef long long long_long_type;
   __extension__ typedef unsigned long long ulong_long_type;
#  else
   typedef long long long_long_type;
   typedef unsigned long long ulong_long_type;
#  endif
}
#endif

// GECODE_BOOST_[APPEND_]EXPLICIT_TEMPLATE_[NON_]TYPE macros --------------------------//
//
// Some compilers have problems with function templates whose template
// parameters don't appear in the function parameter list (basically
// they just link one instantiation of the template in the final
// executable). These macros provide a uniform way to cope with the
// problem with no effects on the calling syntax.

// Example:
//
//  #include <iostream>
//  #include <ostream>
//  #include <typeinfo>
//
//  template <int n>
//  void f() { std::cout << n << ' '; }
//
//  template <typename T>
//  void g() { std::cout << typeid(T).name() << ' '; }
//
//  int main() {
//    f<1>();
//    f<2>();
//
//    g<int>();
//    g<double>();
//  }
//
// With VC++ 6.0 the output is:
//
//   2 2 double double
//
// To fix it, write
//
//   template <int n>
//   void f(GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE(int, n)) { ... }
//
//   template <typename T>
//   void g(GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE(T)) { ... }
//


#if defined(GECODE_BOOST_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS) && defined(__cplusplus)

#  include "gecode/third-party/boost/type.hpp"
#  include "gecode/third-party/boost/non_type.hpp"

#  define GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE(t)              gecode_boost::type<t>* = 0
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE_SPEC(t)         gecode_boost::type<t>*
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE(t, v)       gecode_boost::non_type<t, v>* = 0
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)  gecode_boost::non_type<t, v>*

#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(t)        \
             , GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE(t)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)   \
             , GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v) \
             , GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)    \
             , GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#else

// no workaround needed: expand to nothing

#  define GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE(t)
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define GECODE_BOOST_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(t)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define GECODE_BOOST_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)


#endif // defined GECODE_BOOST_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS

// When GECODE_BOOST_NO_STD_TYPEINFO is defined, we can just import
// the global definition into std namespace:
#if defined(GECODE_BOOST_NO_STD_TYPEINFO) && defined(__cplusplus)
#include <typeinfo>
namespace std{ using ::type_info; }
#endif

// ---------------------------------------------------------------------------//

//
// Helper macro GECODE_BOOST_STRINGIZE:
// Converts the parameter X to a string after macro replacement
// on X has been performed.
//
#define GECODE_BOOST_STRINGIZE(X) GECODE_BOOST_DO_STRINGIZE(X)
#define GECODE_BOOST_DO_STRINGIZE(X) #X

//
// Helper macro GECODE_BOOST_JOIN:
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in GECODE_BOOST_DO_JOIN2 but does in GECODE_BOOST_DO_JOIN.
//
#define GECODE_BOOST_JOIN( X, Y ) GECODE_BOOST_DO_JOIN( X, Y )
#define GECODE_BOOST_DO_JOIN( X, Y ) GECODE_BOOST_DO_JOIN2(X,Y)
#define GECODE_BOOST_DO_JOIN2( X, Y ) X##Y

//
// Set some default values for compiler/library/platform names.
// These are for debugging config setup only:
//
#  ifndef GECODE_BOOST_COMPILER
#     define GECODE_BOOST_COMPILER "Unknown ISO C++ Compiler"
#  endif
#  ifndef GECODE_BOOST_STDLIB
#     define GECODE_BOOST_STDLIB "Unknown ISO standard library"
#  endif
#  ifndef GECODE_BOOST_PLATFORM
#     if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) \
         || defined(_POSIX_SOURCE)
#        define GECODE_BOOST_PLATFORM "Generic Unix"
#     else
#        define GECODE_BOOST_PLATFORM "Unknown"
#     endif
#  endif

//
// Set some default values GPU support
//
#  ifndef GECODE_BOOST_GPU_ENABLED
#  define GECODE_BOOST_GPU_ENABLED
#  endif

//
// constexpr workarounds
//
#if defined(GECODE_BOOST_NO_CONSTEXPR)
#define GECODE_BOOST_CONSTEXPR
#define GECODE_BOOST_CONSTEXPR_OR_CONST const
#else
#define GECODE_BOOST_CONSTEXPR constexpr
#define GECODE_BOOST_CONSTEXPR_OR_CONST constexpr
#endif

#define GECODE_BOOST_STATIC_CONSTEXPR  static GECODE_BOOST_CONSTEXPR_OR_CONST

#endif

