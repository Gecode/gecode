/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/support.hh>

#ifdef GECODE_HAS_FAULT_INJECTION

namespace Gecode { namespace Support { namespace FailPoint {

  namespace {
    std::atomic<unsigned long long> fail_after_count;
    std::atomic<unsigned long long> seen_count;
    std::atomic<int> active_phase;
  }

  void reset(void) {
    active_phase.store(static_cast<int>(Phase::Disabled),
                       std::memory_order_release);
    fail_after_count.store(0, std::memory_order_release);
    seen_count.store(0, std::memory_order_release);
  }

  void fail_after(Phase p, unsigned long long n) {
    seen_count.store(0, std::memory_order_release);
    fail_after_count.store(n, std::memory_order_release);
    active_phase.store(static_cast<int>(p), std::memory_order_release);
  }

  void check(Phase p) {
    if (active_phase.load(std::memory_order_acquire) != static_cast<int>(p))
      return;
    unsigned long long seen =
      seen_count.fetch_add(1, std::memory_order_acq_rel);
    if (seen >= fail_after_count.load(std::memory_order_acquire))
      throw MemoryExhausted();
  }

  unsigned long long count(void) {
    return seen_count.load(std::memory_order_acquire);
  }

}}}

#endif

// STATISTICS: support-any
