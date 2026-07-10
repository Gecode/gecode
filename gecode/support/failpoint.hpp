/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#ifndef GECODE_SUPPORT_FAILPOINT_HPP
#define GECODE_SUPPORT_FAILPOINT_HPP

#include <atomic>

/**
 * \brief Internal, test-only deterministic fault injection support
 *
 * FailPoint state is process-global.  This API is supported only by the
 * dedicated isolated single-threaded fault-test executable; it is not a
 * supported application API and must not be used for concurrent execution.
 */
namespace Gecode { namespace Support { namespace FailPoint {

  /// Named phase for deterministic test failure injection
  enum class Phase {
    Disabled,
    Heap,
    PropagatorCopy,
    BrancherCopy,
    AdvisorCopy,
    DerivedSpaceCopy,
    LocalObjectCopy,
    SpaceDisposeNoticeArray,
    SpaceDisposalArray,
    IntSet,
    MiniModel
  };

  /// Reset all failpoint state
  GECODE_SUPPORT_EXPORT void reset(void);
  /// Fail after \a n successful checks for phase \a p
  GECODE_SUPPORT_EXPORT void fail_after(Phase p, unsigned long long n);
  /// Check failpoint for phase \a p
  GECODE_SUPPORT_EXPORT void check(Phase p);
  /// Number of checks observed for the configured phase
  GECODE_SUPPORT_EXPORT unsigned long long count(void);

}}}

#endif

// STATISTICS: support-any
