/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#include <atomic>

namespace Gecode { namespace Support {

  /// A class for thread-safe reference counting
  class RefCount {
  protected:
    /// The reference count
    std::atomic_ulong n;
  public:
    /// Initialize
    RefCount(unsigned long int n);
    /// Increment counter
    void inc(void);
    /// Decrement counter and return true if it hits zero
    bool dec(void);
    /// Test whether reference count is non-zero
    operator bool(void) const;
  };

  forceinline
  RefCount::RefCount(unsigned long int n0) {
    n.store(n0,std::memory_order_release);
  }
  forceinline void
  RefCount::inc(void) {
    (void) n.fetch_add(1U,std::memory_order_relaxed);
  }
  forceinline bool
  RefCount::dec(void) {
    return n.fetch_sub(1U,std::memory_order_acq_rel) == 1U;
  }
  forceinline
  RefCount::operator bool(void) const {
    return n.load(std::memory_order_acquire) > 0U;
  }

}}

// STATISTICS: support-any
