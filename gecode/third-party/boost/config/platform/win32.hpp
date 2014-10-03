//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Bill Kempf 2001. 
//  (C) Copyright Aleksey Gurtovoy 2003. 
//  (C) Copyright Rene Rivera 2005.
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Win32 specific config options:

#define GECODE_BOOST_PLATFORM "Win32"

//  Get the information about the MinGW runtime, i.e. __MINGW32_*VERSION.
#if defined(__MINGW32__)
#  include <_mingw.h>
#endif

#if defined(__GNUC__) && !defined(GECODE_BOOST_NO_SWPRINTF)
#  define GECODE_BOOST_NO_SWPRINTF
#endif

//  Default defines for GECODE_BOOST_SYMBOL_EXPORT and GECODE_BOOST_SYMBOL_IMPORT
//  If a compiler doesn't support __declspec(dllexport)/__declspec(dllimport),
//  its boost/config/compiler/ file must define GECODE_BOOST_SYMBOL_EXPORT and
//  GECODE_BOOST_SYMBOL_IMPORT
#ifndef GECODE_BOOST_SYMBOL_EXPORT
#  define GECODE_BOOST_HAS_DECLSPEC
#  define GECODE_BOOST_SYMBOL_EXPORT __declspec(dllexport)
#  define GECODE_BOOST_SYMBOL_IMPORT __declspec(dllimport)
#endif

#if defined(__MINGW32__) && ((__MINGW32_MAJOR_VERSION > 2) || ((__MINGW32_MAJOR_VERSION == 2) && (__MINGW32_MINOR_VERSION >= 0)))
#  define GECODE_BOOST_HAS_STDINT_H
#  define __STDC_LIMIT_MACROS
#  define GECODE_BOOST_HAS_DIRENT_H
#  define GECODE_BOOST_HAS_UNISTD_H
#endif

#if defined(__MINGW32__) && (__GNUC__ >= 4)
#  define GECODE_BOOST_HAS_EXPM1
#  define GECODE_BOOST_HAS_LOG1P
#  define GECODE_BOOST_HAS_GETTIMEOFDAY
#endif
//
// Win32 will normally be using native Win32 threads,
// but there is a pthread library avaliable as an option,
// we used to disable this when GECODE_BOOST_DISABLE_WIN32 was 
// defined but no longer - this should allow some
// files to be compiled in strict mode - while maintaining
// a consistent setting of GECODE_BOOST_HAS_THREADS across
// all translation units (needed for shared_ptr etc).
//

#ifdef _WIN32_WCE
#  define GECODE_BOOST_NO_ANSI_APIS
#endif

#ifndef GECODE_BOOST_HAS_PTHREADS
#  define GECODE_BOOST_HAS_WINTHREADS
#endif

#ifndef GECODE_BOOST_DISABLE_WIN32
// WEK: Added
#define GECODE_BOOST_HAS_FTIME
#define GECODE_BOOST_WINDOWS 1

#endif
