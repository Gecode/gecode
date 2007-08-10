/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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



#include "gecode/set/int.hh"

#include "gecode/iter.hh"

using namespace Gecode::Int;

namespace Gecode { namespace Set { namespace Int {

  PropCost
  Weights::cost(void) const {
    return PC_LINEAR_LO;
  }

  std::string
  Weights::name(void) {
    return std::string("Set::Int::Weights");
  }

  size_t
  Weights::dispose(Space* home) {
    assert(!home->failed());
    x.cancel(home,this, PC_SET_ANY);
    y.cancel(home,this, Gecode::Int::PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  Actor*
  Weights::copy(Space* home, bool share) {
    return new (home) Weights(home,share,*this);
  }

  enum CostType { POS_COST, NEG_COST, ALL_COST };

  template <class I, CostType c>
  forceinline
  int weightI(Support::SharedArray<int>& elements,
              Support::SharedArray<int>& weights,
              I& iter) {
    int sum = 0;
    int i = 0;
    for (Iter::Ranges::ToValues<I> v(iter); v(); ++v) {
      // Skip all elements below the current
      while (elements[i]<v.val()) i++;
      assert(elements[i] == v.val());
      switch (c) {
      case ALL_COST:
        sum += weights[i];
        break;
      case POS_COST:
        if (weights[i] > 0) sum += weights[i];
        break;
      case NEG_COST:
        if (weights[i] < 0) sum += weights[i];
        break;
      default: GECODE_NEVER;
      }
    }
    return sum;
  }


  /// Sort order for integers
  class IntLt {
  public:
    bool operator()(int x, int y);
  };

  forceinline bool 
  IntLt::operator()(int x, int y) {
    return x < y;
  }

  ExecStatus
  Weights::propagate(Space* home) {

    if (x.assigned()) {
      GlbRanges<SetView> glb(x);
      int w = 
        weightI<GlbRanges<SetView>,ALL_COST>(elements, weights, glb);
      GECODE_ME_CHECK(y.eq(home, w));
      return ES_SUBSUMED(this,home);
    }

    int lowCost;
    int lowestCost;
    int highestCost;
    int size = elements.size();
    {
      UnknownRanges<SetView> ur(x);
      Iter::Ranges::ToValues<UnknownRanges<SetView> > urv(ur);
      GECODE_AUTOARRAY(int, currentWeights, size);
      for (int i=0; i<size; i++) {
        if (!urv() || elements[i]<urv.val()) {
          currentWeights[i] = 0;
        } else {
          assert(elements[i] == urv.val());
          currentWeights[i] = weights[i];
          ++urv;
        }
      }

      IntLt ilt;
      Support::quicksort<int>(currentWeights, size, ilt);

      GlbRanges<SetView> glb(x);
      lowCost = weightI<GlbRanges<SetView>,ALL_COST>(elements, weights, glb);
      highestCost =
        weightI<GlbRanges<SetView>,ALL_COST>(elements, weights, glb);

      int delta = std::min(x.unknownSize(), x.cardMax() - x.glbSize());

      for (int i=0; i<delta-1; i++) {
        if (currentWeights[i] >= 0)
          break;
        lowCost+=currentWeights[i];
      }
      lowestCost = lowCost;
      if (delta>0 && currentWeights[delta-1]<0)
        lowestCost+=currentWeights[delta-1];

      for (int i=0; i<delta; i++) {
        if (currentWeights[size-i-1]<=0)
          break;
        highestCost += currentWeights[size-i-1];
      }

    }

    GECODE_ME_CHECK(y.gq(home, lowestCost));
    GECODE_ME_CHECK(y.lq(home, highestCost));

    ModEvent me;
    {
      UnknownRanges<SetView> ur2(x);
      Iter::Ranges::ToValues<UnknownRanges<SetView> > urv(ur2);
      OverweightValues<Iter::Ranges::ToValues<UnknownRanges<SetView> > >
        ov(y.max()-lowCost, elements, weights, urv);
      Iter::Values::ToRanges<OverweightValues<
        Iter::Ranges::ToValues<UnknownRanges<SetView> > > > ovr(ov);
      me = x.excludeI(home, ovr);
      GECODE_ME_CHECK(me);
    }

    if (x.assigned()) {
      GlbRanges<SetView> glb(x);
      int w = 
        weightI<GlbRanges<SetView>,ALL_COST>(elements, weights, glb);
      GECODE_ME_CHECK(y.eq(home, w));
      return ES_SUBSUMED(this,home);
    }
    return me_modified(me) ? ES_NOFIX : ES_FIX;
  }

  Reflection::ActorSpec&
  Weights::spec(Space* home, Reflection::VarMap& m) {
    Reflection::ActorSpec& s = Propagator::spec(home, m, name());
    return s << new Reflection::ArrayArg<int>(elements)
             << new Reflection::ArrayArg<int>(weights)
             << Reflection::typedSpec(home, m, x)
             << Reflection::typedSpec(home, m, y);
  }

}}}

// STATISTICS: set-prop
