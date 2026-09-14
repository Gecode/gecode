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
 */

#ifndef GECODE_WORD_CONDITIONAL_HH
#define GECODE_WORD_CONDITIONAL_HH

#include <gecode/word/rel.hh>

namespace Gecode { namespace Word { namespace Conditional {

  /** \brief Boolean-controlled word if-then-else propagator
   *
   * With an unknown control, the result is narrowed to the cube hull of both
   * branches and a branch that is disjoint from the result is rejected.
   */
  template<class V0, class V1, class V2>
  class Ite : public Propagator {
  protected:
    Int::BoolView b;
    V0 x0;
    V1 x1;
    V2 x2;
    Ite(Home home, Int::BoolView b, V0 x0, V1 x1, V2 x2);
    Ite(Space& home, Ite& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, Int::BoolView b,
                           V0 x0, V1 x1, V2 x2);
  };

  /** \brief Boolean-controlled bounded word if-then-else propagator
   *
   * With an unknown control, the result is narrowed to the cube hull and
   * ranked interval hull of both branches. A branch whose cube and ranked
   * interval are disjoint from the result is rejected.
   */
  template<class View>
  class BoundIte : public Propagator {
  protected:
    Int::BoolView b;
    View x0;
    View x1;
    View x2;
    BoundIte(Home home, Int::BoolView b, View x0, View x1, View x2);
    BoundIte(Space& home, BoundIte& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, Int::BoolView b,
                           View x0, View x1, View x2);
  };

}}}

#include <gecode/word/conditional/ite.hpp>

#endif

// STATISTICS: word-prop
