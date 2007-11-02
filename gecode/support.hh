/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __GECODE_SUPPORT_HH__
#define __GECODE_SUPPORT_HH__

/*
 * Support for DLLs under Windows
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

/**
  * \brief Workaround for a bug in the Microsoft C++ compiler
  *
  * Details for the bug can be found in
  * http://support.microsoft.com/?scid=kb%3Ben-us%3B122675&x=9&y=13
  */
#define MSCVIRTUAL virtual

#ifdef GECODE_BUILD_SUPPORT
#define GECODE_SUPPORT_EXPORT __declspec( dllexport )
#else
#define GECODE_SUPPORT_EXPORT __declspec( dllimport )
#endif

#else

/**
  * \brief Workaround for a bug in the Microsoft C++ compiler
  *
  * Details for the bug can be found in
  * http://support.microsoft.com/?scid=kb%3Ben-us%3B122675&x=9&y=13
  */
#define MSCVIRTUAL

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_SUPPORT_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_SUPPORT_EXPORT

#endif
#endif

#include <cassert>

#include "gecode/config.hh"

/*
 * Basic support needed everywhere
 *
 */

#include "gecode/support/exception.icc"
#include "gecode/support/cast.icc"
#include "gecode/support/memory.icc"

/*
 * Common datastructures and algorithms
 *
 */

#include "gecode/support/block-allocator.icc"
#include "gecode/support/dynamic-array.icc"
#include "gecode/support/dynamic-stack.icc"
#include "gecode/support/random.icc"
#include "gecode/support/search.icc"
#include "gecode/support/sentinel-stack.icc"
#include "gecode/support/sort.icc"
#include "gecode/support/static-pqueue.icc"
#include "gecode/support/static-stack.icc"
#include "gecode/support/symbol.icc"
#include "gecode/support/map.icc"

#endif

// STATISTICS: support-any
