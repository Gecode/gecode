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

#ifndef GECODE_WORD_REL_HH
#define GECODE_WORD_REL_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Rel {

  /// Test whether two views refer to the same word variable
  template<class View0, class View1>
  bool aliases(View0 x, View1 y);

  /// Test whether the domains of \a x and \a y are disjoint
  template<class View0, class View1>
  bool disjoint(View0 x, View1 y);

  /// Test the equality relation
  template<class View0, class View1>
  Int::RelTest eq_test(View0 x, View1 y);

  /// Word-level bit-consistent equality propagator
  template<class View0, class View1>
  class Eq : public MixBinaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x1;
    Eq(Home home, View0 x0, View1 x1);
    Eq(Space& home, Eq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Word disequality propagator
  template<class View0, class View1>
  class Nq : public MixBinaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x1;
    Nq(Home home, View0 x0, View1 x1);
    Nq(Space& home, Nq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Reified word equality propagator
  template<class View0, class View1, class CtrlView, ReifyMode rm>
  class ReEq : public Propagator {
  protected:
    View0 x0;
    View1 x1;
    CtrlView b;
    ReEq(Home home, View0 x0, View1 x1, CtrlView b);
    ReEq(Space& home, ReEq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1, CtrlView b);
  };

}}}

#include <gecode/word/rel/eq.hpp>
#include <gecode/word/rel/nq.hpp>
#include <gecode/word/rel/re-eq.hpp>

#endif

// STATISTICS: word-prop
