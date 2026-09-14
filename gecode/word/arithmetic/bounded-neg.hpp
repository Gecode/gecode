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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_NEG_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_NEG_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-rank.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline WordRankInterval
  project_negation_interval(WordRankInterval interval, WordValue mask) {
    return {mask-interval.maximum+1,mask-interval.minimum+1};
  }

  /// Apply signed negation and derive its inverse from the narrowed result.
  template<class View>
  forceinline bool
  narrow_bound_neg_ranges(BoundLocalDomain& x, BoundLocalDomain& z) {
    if (!View::signed_order) return true;
    if (x.minimum == 0) return true;
    const WordValue mask=width_mask(x.width);
    const WordRankInterval forward=project_negation_interval(
      snapshot_bound_interval(x),mask);
    if (!z.intersect_range(forward.minimum,forward.maximum)) return false;
    const WordRankInterval inverse=project_negation_interval(
      snapshot_bound_interval(z),mask);
    return x.intersect_range(inverse.minimum,inverse.maximum);
  }

  /// Staged local cube and interval filtering for signed negation.
  template<class View>
  class BoundNeg : public BinaryPropagator<View,PC_WORD_DOM> {
  public:
    static bool numeric_regime(View x) { return x.rank_minimum() != 0U; }
    virtual Actor* copy(Space& home) { return new (home) BoundNeg(home,*this); }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::binary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        const BoundFilterResult bounds=narrow_bounds(home,x0,x1);
        GECODE_ES_CHECK(bounds.status);
        const bool is_assigned=x0.assigned() && x1.assigned();
        if (is_assigned)
          return home.ES_SUBSUMED(*this);
        if (bounds.needs_cube)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      GECODE_ES_CHECK(narrow(home,x0,x1,true));
      const bool is_assigned=x0.assigned() && x1.assigned();
      return is_assigned ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View z) {
      GECODE_ES_CHECK(narrow(home,x,z,true));
      const bool is_assigned=x.assigned() && z.assigned();
      if (!is_assigned)
        (void) new (home) BoundNeg(home,x,z);
      return ES_OK;
    }
  protected:
    using BinaryPropagator<View,PC_WORD_DOM>::x0;
    using BinaryPropagator<View,PC_WORD_DOM>::x1;
    BoundNeg(Home home, View x, View z)
      : BinaryPropagator<View,PC_WORD_DOM>(home,x,z) {}
    BoundNeg(Space& home, BoundNeg& p)
      : BinaryPropagator<View,PC_WORD_DOM>(home,p) {}
    static BoundFilterResult narrow_bounds(Home home, View x, View z) {
      const View input[2]={x,z};
      BoundLocalPass<View,2> pass(input);
      const BoundCubeSnapshot<2> initial=pass.snapshot_bits();
      for (;;) {
        const BoundDomainSnapshot<2> previous=pass.snapshot();
        pass.defer_synchronization();
        if (!narrow_bound_neg_ranges<View>(pass.domain(0),pass.domain(1)))
          return BoundFilterResult{ES_FAILED,false};
        if (!pass.synchronize_changed(previous))
          return BoundFilterResult{ES_FAILED,false};
        const bool is_stable=pass.is_unchanged(previous);
        if (is_stable) break;
      }
      const bool has_new_bits=pass.has_new_bits(initial);
      return BoundFilterResult{pass.publish(home),has_new_bits};
    }
    static ExecStatus narrow(Home home, View x, View z, bool needs_cube) {
      const View input[2]={x,z};
      BoundLocalPass<View,2> pass(input);
      for (;;) {
        const BoundDomainSnapshot<2> previous=pass.snapshot();
        pass.defer_synchronization();
        if (needs_cube)
          GECODE_ES_CHECK(narrow_negation(home,pass.view(0),pass.view(1)));
        const BoundCubeSnapshot<2> before_ranges=pass.snapshot_bits();
        if (!narrow_bound_neg_ranges<View>(pass.domain(0),pass.domain(1)))
          return ES_FAILED;
        if (!pass.synchronize_changed(previous)) return ES_FAILED;
        needs_cube=pass.has_new_bits(before_ranges);
        const bool is_stable=pass.is_unchanged(previous);
        if (is_stable) break;
      }
      return pass.publish(home);
    }
  };

}}}

#endif
