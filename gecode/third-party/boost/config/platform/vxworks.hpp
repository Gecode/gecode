//  (C) Copyright Dustin Spicuzza 2009. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  vxWorks specific config options:

#define GECODE_BOOST_PLATFORM "vxWorks"

#define GECODE_BOOST_NO_CWCHAR
#define GECODE_BOOST_NO_INTRINSIC_WCHAR_T

#if defined(__GNUC__) && defined(__STRICT_ANSI__)
#define GECODE_BOOST_NO_INT64_T
#endif

#define GECODE_BOOST_HAS_UNISTD_H

// these allow posix_features to work, since vxWorks doesn't
// define them itself
#define _POSIX_TIMERS 1
#define _POSIX_THREADS 1

// vxworks doesn't work with asio serial ports
#define GECODE_BOOST_ASIO_DISABLE_SERIAL_PORT

// boilerplate code:
#include <gecode/third-party/boost/config/posix_features.hpp>
 
