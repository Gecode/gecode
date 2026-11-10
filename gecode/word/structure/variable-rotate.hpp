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

namespace Gecode { namespace Word { namespace Structure {

  forceinline
  VariableRotation::VariableRotation(Home home, WordView input, WordView count,
                                      WordView result, FixedOp direction)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,input,count,result),
      op(direction) {}

  forceinline
  VariableRotation::VariableRotation(Space& home, VariableRotation& actor)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,actor), op(actor.op) {}

  inline ExecStatus
  VariableRotation::post(Home home, WordView input, WordView count,
                         WordView result, FixedOp op) {
    if (count.assigned())
      return Fixed<WordView,WordView>::post(home,input,result,op,reduce_count(count));
    const RotationViews views={input,count,result};
    const RotationClosureResult closure=close_rotation(home,views,op);
    GECODE_ES_CHECK(closure.status);
    if (has_single_residue(closure.reachable))
      return Fixed<WordView,WordView>::post(home,input,result,op,
                                          find_residue(closure.reachable));
    if (!is_entailed(views.input,views.result,closure))
      (void) new (home) VariableRotation(home,input,count,result,op);
    return ES_OK;
  }

  inline Actor*
  VariableRotation::copy(Space& home) {
    return new (home) VariableRotation(home,*this);
  }

  inline PropCost
  VariableRotation::cost(const Space&, const ModEventDelta&) const {
    if (x1.assigned()) return PropCost::linear(PropCost::LO,x0.width());
    const bool has_bounds=x0.bounded() || x1.bounded() || x2.bounded();
    return has_bounds ? PropCost::cubic(PropCost::LO,x0.width()) :
      PropCost::quadratic(PropCost::LO,x0.width());
  }

  inline size_t
  VariableRotation::dispose(Space& home) {
    (void) TernaryPropagator<WordView,PC_WORD_BITS>::dispose(home);
    return sizeof(*this);
  }

  inline ExecStatus
  VariableRotation::propagate(Space& home, const ModEventDelta&) {
    if (x1.assigned())
      GECODE_REWRITE(*this,(Fixed<WordView,WordView>::post(
        home(*this),x0,x2,op,reduce_count(x1))));
    const RotationViews views={x0,x1,x2};
    const RotationClosureResult closure=close_rotation(home,views,op);
    GECODE_ES_CHECK(closure.status);
    if (has_single_residue(closure.reachable))
      GECODE_REWRITE(*this,(Fixed<WordView,WordView>::post(
        home(*this),x0,x2,op,find_residue(closure.reachable))));
    return is_entailed(views.input,views.result,closure) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
