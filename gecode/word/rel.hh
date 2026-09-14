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

  /**
   * \brief Direct word ordering propagator
   *
   * Propagation applies the lo/hi bound rules of Wang et al. from the most
   * significant bit. The \a sign template argument selects unsigned order or
   * two's-complement signed order.
   */
  template<class View0, class View1, bool sign>
  class Lq : public MixBinaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x1;
    Lq(Home home, View0 x0, View1 x1);
    Lq(Space& home, Lq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Direct strict word ordering propagator
  template<class View0, class View1, bool sign>
  class Le : public MixBinaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x1;
    Le(Home home, View0 x0, View1 x1);
    Le(Space& home, Le& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Reified non-strict word ordering propagator
  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  class ReLq : public Propagator {
  protected:
    View0 x0;
    View1 x1;
    CtrlView b;
    ReLq(Home home, View0 x0, View1 x1, CtrlView b);
    ReLq(Space& home, ReLq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1, CtrlView b);
  };

  /// Equality propagator for matching bounded word domains
  template<class View0, class View1>
  class BoundEq : public MixBinaryPropagator<
    View0,PC_WORD_DOM,View1,PC_WORD_DOM> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_DOM,View1,PC_WORD_DOM>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_DOM,View1,PC_WORD_DOM>::x1;
    BoundEq(Home home, View0 x0, View1 x1);
    BoundEq(Space& home, BoundEq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Disequality propagator for matching bounded word domains
  template<class View>
  class BoundNq : public MixBinaryPropagator<
    View,PC_WORD_DOM,View,PC_WORD_DOM> {
  protected:
    using MixBinaryPropagator<
      View,PC_WORD_DOM,View,PC_WORD_DOM>::x0;
    using MixBinaryPropagator<
      View,PC_WORD_DOM,View,PC_WORD_DOM>::x1;
    BoundNq(Home home, View x0, View x1);
    BoundNq(Space& home, BoundNq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View x0, View x1);
  };

  /// Numeric ordering propagator for matching bounded word domains
  template<class View0, class View1, bool strict>
  class BoundLq : public MixBinaryPropagator<
    View0,PC_WORD_BND,View1,PC_WORD_BND> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BND,View1,PC_WORD_BND>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BND,View1,PC_WORD_BND>::x1;
    BoundLq(Home home, View0 x0, View1 x1);
    BoundLq(Space& home, BoundLq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1);
  };

  /// Reified equality propagator for matching bounded word domains
  template<class View0, class View1, class CtrlView, ReifyMode rm>
  class ReBoundEq : public Propagator {
  protected:
    View0 x0;
    View1 x1;
    CtrlView b;
    ReBoundEq(Home home, View0 x0, View1 x1, CtrlView b);
    ReBoundEq(Space& home, ReBoundEq& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1, CtrlView b);
  };

  /// Reified numeric ordering for matching bounded word domains
  template<class View0, class View1, class CtrlView,
           ReifyMode rm>
  class ReBoundLq : public Propagator {
  protected:
    View0 x0;
    View1 x1;
    CtrlView b;
    ReBoundLq(Home home, View0 x0, View1 x1, CtrlView b);
    ReBoundLq(Space& home, ReBoundLq& p);
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
#include <gecode/word/rel/lq-le.hpp>
#include <gecode/word/rel/re-lq.hpp>
#include <gecode/word/rel/bounded.hpp>

#endif

// STATISTICS: word-prop
