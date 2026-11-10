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

  /// Negate a rank interval whose minimum is at least one
  forceinline WordRankInterval
  project_negation_interval(WordRankInterval interval, WordValue mask) {
    assert(interval.minimum != 0U);
    return {mask-interval.maximum+1U,mask-interval.minimum+1U};
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
      const ExecStatus status=narrow(home,x0,x1);
      GECODE_ES_CHECK(status);
      const bool is_assigned=x0.assigned() && x1.assigned();
      return is_assigned ? home.ES_SUBSUMED(*this) : status;
    }
    static ExecStatus post(Home home, View x, View z) {
      const ExecStatus status=narrow(home,x,z);
      GECODE_ES_CHECK(status);
      const bool is_assigned=x.assigned() && z.assigned();
      if (!is_assigned)
        schedule_unless_fixpoint(home,*new (home) BoundNeg(home,x,z),status);
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
      PassProgress progress;
      // Bounds-only stage: a closure jump stops it and hands over to the cube stage.
      do {
        const BoundDomainSnapshot<2> previous=pass.snapshot();
        pass.defer_synchronization();
        if (!narrow_bound_neg_ranges<View>(pass.domain(0),pass.domain(1)))
          return BoundFilterResult{ES_FAILED,false};
        progress=pass.close_changed(previous);
        if (progress == PassProgress::FAILED)
          return BoundFilterResult{ES_FAILED,false};
      } while (progress == PassProgress::CHANGED);
      return finish_bound_stage(pass_result(pass.publish(home),progress),
                                pass.has_new_bits(initial),
                                [&](void) { return narrow(home,x,z); });
    }
    /// Filter cube and interval until publication closes no value set further
    static ExecStatus narrow(Home home, View x, View z) {
      return repeat_until_closed([&](void) { return narrow_once(home,x,z); });
    }
    /// Filter cube and interval locally to a fixpoint and publish them
    static PassResult narrow_once(Home home, View x, View z) {
      const View input[2]={x,z};
      BoundLocalPass<View,2> pass(input);
      bool needs_cube=true;
      PassProgress progress;
      // Expensive pass: a closure jump stops it, so cheaper propagators run first.
      do {
        const BoundDomainSnapshot<2> previous=pass.snapshot();
        pass.defer_synchronization();
        const bool has_failed_cube=needs_cube &&
          (narrow_negation(home,pass.view(0),pass.view(1)) < ES_OK);
        if (has_failed_cube) return PassResult::FAILED;
        const BoundCubeSnapshot<2> before_ranges=pass.snapshot_bits();
        if (!narrow_bound_neg_ranges<View>(pass.domain(0),pass.domain(1)))
          return PassResult::FAILED;
        progress=pass.close_changed(previous);
        if (progress == PassProgress::FAILED) return PassResult::FAILED;
        needs_cube=pass.has_new_bits(before_ranges);
      } while (progress == PassProgress::CHANGED);
      return pass_result(pass.publish(home),progress);
    }
  };

}}}

#endif
