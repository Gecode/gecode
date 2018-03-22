/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <cstring>
#include <cstdlib>

#ifdef GECODE_PEAKHEAP_MALLOC_H
#include <malloc.h>
#endif

#ifdef GECODE_PEAKHEAP_MALLOC_MALLOC_H
#include <malloc/malloc.h>
#endif

#ifdef GECODE_ALLOCATOR

namespace Gecode { namespace Support {

  /**
   * \brief Default memory allocator
   *
   * Acts as a wrapper around memory allocation functionality.
   *
   * Can be disabled by giving a "-disable-allocator" option to
   * configure and the be re-implemented before including any other
   * Gecode header.
   *
   * \ingroup FuncMem
   */
  class Allocator {
  public:
    /// Default constructor
    Allocator(void);
    /// Allocate memory block of size \a n
    void* alloc(size_t n);
    /// Return address of reallocated memory block \a p of size \a n
    void* realloc(void* p, size_t n);
    /// Free memory block \a p
    void free(void* p);
    /// Copy \a n bytes from source \a s directly to \a d and returns \a d
    void* memcpy(void *d, const void *s, size_t n);
  };


  forceinline
  Allocator::Allocator(void) {
  }
  forceinline void*
  Allocator::alloc(size_t n) {
    return ::malloc(n);
  }
  forceinline void*
  Allocator::realloc(void* p, size_t n) {
    return ::realloc(p,n);
  }
  forceinline void
  Allocator::free(void* p) {
    ::free(p);
  }
  forceinline void*
  Allocator::memcpy(void *d, const void *s, size_t n) {
    return ::memcpy(d,s,n);
  }

}}

#endif

namespace Gecode { namespace Support {

  /**
   * \brief The single global default memory allocator
   * \ingroup FuncMem
   */
  extern GECODE_SUPPORT_EXPORT
  Allocator allocator;

}}

// STATISTICS: support-any
