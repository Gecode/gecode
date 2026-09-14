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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_BINARY_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_BINARY_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-rank.hpp>
#include <gecode/word/arithmetic/bounded-terminal.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Both inverse intervals, computed before either operand is narrowed.
  struct BoundOperandIntervals {
    WordRankInterval x, y;
  };

  template<class View>
  forceinline BoundIntervalResult
  project_add_interval(WordRankInterval x, WordRankInterval y,
                       unsigned int width) {
    const WordValue mask=width_mask(width);
    if (!View::signed_order) {
      if (x.maximum > mask-y.maximum) return {false,{0,0}};
      return {true,{x.minimum+y.minimum,x.maximum+y.maximum}};
    }
    const WordValue sign=sign_bit(width);
    const BoundRankResult minimum=add_signed_ranks(x.minimum,y.minimum,sign,mask);
    if (!minimum.is_valid) return {false,{0,0}};
    const BoundRankResult maximum=add_signed_ranks(x.maximum,y.maximum,sign,mask);
    if (!maximum.is_valid) return {false,{0,0}};
    return {true,{minimum.value,maximum.value}};
  }

  template<class View>
  forceinline BoundOperandIntervals
  project_add_operands(WordRankInterval x, WordRankInterval y,
                       WordRankInterval z, unsigned int width) {
    const WordValue mask=width_mask(width);
    WordValue xmin, xmax, ymin, ymax;
    if (!View::signed_order) {
      xmin=(z.minimum >= y.maximum) ? z.minimum-y.maximum : 0;
      xmax=z.maximum-y.minimum;
      ymin=(z.minimum >= x.maximum) ? z.minimum-x.maximum : 0;
      ymax=z.maximum-x.minimum;
    } else {
      const WordValue sign=sign_bit(width);
      const BoundRankResult minimum_x=subtract_signed_ranks(
        z.minimum,y.maximum,sign,mask);
      xmin=minimum_x.is_valid ? minimum_x.value : 0;
      const BoundRankResult maximum_x=subtract_signed_ranks(
        z.maximum,y.minimum,sign,mask);
      xmax=maximum_x.is_valid ? maximum_x.value : mask;
      const BoundRankResult minimum_y=subtract_signed_ranks(
        z.minimum,x.maximum,sign,mask);
      ymin=minimum_y.is_valid ? minimum_y.value : 0;
      const BoundRankResult maximum_y=subtract_signed_ranks(
        z.maximum,x.minimum,sign,mask);
      ymax=maximum_y.is_valid ? maximum_y.value : mask;
    }
    return {{std::max(x.minimum,xmin),std::min(x.maximum,xmax)},
            {std::max(y.minimum,ymin),std::min(y.maximum,ymax)}};
  }

  /// Apply forward addition, then derive both inverse intervals from live roles.
  template<class View>
  forceinline bool
  narrow_bound_add_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                          BoundLocalDomain& z) {
    const BoundIntervalResult forward=project_add_interval<View>(
      snapshot_bound_interval(x),snapshot_bound_interval(y),x.width);
    if (!forward.has_interval) return true;
    if (!z.intersect_range(forward.interval.minimum,forward.interval.maximum))
      return false;
    const BoundOperandIntervals inverse=project_add_operands<View>(
      snapshot_bound_interval(x),snapshot_bound_interval(y),
      snapshot_bound_interval(z),x.width);
    return x.intersect_range(inverse.x.minimum,inverse.x.maximum) &&
      y.intersect_range(inverse.y.minimum,inverse.y.maximum);
  }

  template<class View>
  forceinline BoundIntervalResult
  project_sub_interval(WordRankInterval x, WordRankInterval y,
                       unsigned int width) {
    const WordValue mask=width_mask(width);
    if (!View::signed_order) {
      if (x.minimum < y.maximum) return {false,{0,0}};
      return {true,{x.minimum-y.maximum,x.maximum-y.minimum}};
    }
    const WordValue sign=sign_bit(width);
    const BoundRankResult minimum=subtract_signed_ranks(
      x.minimum,y.maximum,sign,mask);
    if (!minimum.is_valid) return {false,{0,0}};
    const BoundRankResult maximum=subtract_signed_ranks(
      x.maximum,y.minimum,sign,mask);
    if (!maximum.is_valid) return {false,{0,0}};
    return {true,{minimum.value,maximum.value}};
  }

  template<class View>
  forceinline BoundOperandIntervals
  project_sub_operands(WordRankInterval x, WordRankInterval y,
                       WordRankInterval z, unsigned int width) {
    const WordValue mask=width_mask(width);
    WordValue xmin, xmax, ymin, ymax;
    if (!View::signed_order) {
      xmin=z.minimum+y.minimum;
      xmax=(z.maximum > mask-y.maximum) ? mask : z.maximum+y.maximum;
      ymin=(x.minimum >= z.maximum) ? x.minimum-z.maximum : 0;
      ymax=x.maximum-z.minimum;
    } else {
      const WordValue sign=sign_bit(width);
      const BoundRankResult minimum_x=add_signed_ranks(
        z.minimum,y.minimum,sign,mask);
      xmin=minimum_x.is_valid ? minimum_x.value : 0;
      const BoundRankResult maximum_x=add_signed_ranks(
        z.maximum,y.maximum,sign,mask);
      xmax=maximum_x.is_valid ? maximum_x.value : mask;
      const BoundRankResult minimum_y=subtract_signed_ranks(
        x.minimum,z.maximum,sign,mask);
      ymin=minimum_y.is_valid ? minimum_y.value : 0;
      const BoundRankResult maximum_y=subtract_signed_ranks(
        x.maximum,z.minimum,sign,mask);
      ymax=maximum_y.is_valid ? maximum_y.value : mask;
    }
    return {{std::max(x.minimum,xmin),std::min(x.maximum,xmax)},
            {std::max(y.minimum,ymin),std::min(y.maximum,ymax)}};
  }

  /// Apply forward subtraction before reading the inverse operand bounds.
  template<class View>
  forceinline bool
  narrow_bound_sub_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                          BoundLocalDomain& z) {
    const BoundIntervalResult forward=project_sub_interval<View>(
      snapshot_bound_interval(x),snapshot_bound_interval(y),x.width);
    if (!forward.has_interval) return true;
    if (!z.intersect_range(forward.interval.minimum,forward.interval.maximum))
      return false;
    const BoundOperandIntervals inverse=project_sub_operands<View>(
      snapshot_bound_interval(x),snapshot_bound_interval(y),
      snapshot_bound_interval(z),x.width);
    return x.intersect_range(inverse.x.minimum,inverse.x.maximum) &&
      y.intersect_range(inverse.y.minimum,inverse.y.maximum);
  }

  forceinline BoundIntervalResult
  project_unsigned_product_interval(WordRankInterval x, WordRankInterval y,
                                    WordValue mask) {
    const bool has_overflow=(x.maximum != 0) && (y.maximum > mask/x.maximum);
    if (has_overflow) return {false,{0,0}};
    return {true,{x.minimum*y.minimum,x.maximum*y.maximum}};
  }

  forceinline BoundOperandIntervals
  project_unsigned_product_operands(WordRankInterval x, WordRankInterval y,
                                    WordRankInterval z) {
    WordValue xmin=x.minimum, xmax=x.maximum;
    WordValue ymin=y.minimum, ymax=y.maximum;
    if (y.maximum != 0)
      xmin=std::max(xmin,z.minimum/y.maximum+((z.minimum%y.maximum) != 0));
    if (y.minimum != 0) xmax=std::min(xmax,z.maximum/y.minimum);
    if (x.maximum != 0)
      ymin=std::max(ymin,z.minimum/x.maximum+((z.minimum%x.maximum) != 0));
    if (x.minimum != 0) ymax=std::min(ymax,z.maximum/x.minimum);
    return {{xmin,xmax},{ymin,ymax}};
  }

  /// Preserve post-result reads while applying nonwrapping product deductions.
  forceinline bool
  narrow_bound_unsigned_product(BoundLocalDomain& x, BoundLocalDomain& y,
                                BoundLocalDomain& z) {
    const BoundIntervalResult forward=project_unsigned_product_interval(
      snapshot_bound_interval(x),snapshot_bound_interval(y),
      width_mask(x.width));
    if (!forward.has_interval) return true;
    if (!z.intersect_range(forward.interval.minimum,forward.interval.maximum))
      return false;
    const BoundOperandIntervals inverse=project_unsigned_product_operands(
      snapshot_bound_interval(x),snapshot_bound_interval(y),
      snapshot_bound_interval(z));
    return x.intersect_range(inverse.x.minimum,inverse.x.maximum) &&
      y.intersect_range(inverse.y.minimum,inverse.y.maximum);
  }

  forceinline BoundIntervalResult
  project_signed_product_interval(WordRankInterval x, WordRankInterval y,
                                  unsigned int width) {
    const WordValue mask=width_mask(width), sign=sign_bit(width);
    const WordValue x_endpoints[2]={x.minimum,x.maximum};
    const WordValue y_endpoints[2]={y.minimum,y.maximum};
    WordValue minimum=mask, maximum=0;
    for (unsigned int i=0; i<2; i++)
      for (unsigned int j=0; j<2; j++) {
        const BoundRankResult product=multiply_signed_ranks(
          x_endpoints[i],y_endpoints[j],sign,mask);
        if (!product.is_valid) return {false,{0,0}};
        minimum=std::min(minimum,product.value);
        maximum=std::max(maximum,product.value);
      }
    return {true,{minimum,maximum}};
  }

  forceinline bool
  narrow_bound_signed_product(BoundLocalDomain& x, BoundLocalDomain& y,
                              BoundLocalDomain& z) {
    const BoundIntervalResult forward=project_signed_product_interval(
      snapshot_bound_interval(x),snapshot_bound_interval(y),x.width);
    if (!forward.has_interval) return true;
    return z.intersect_range(forward.interval.minimum,forward.interval.maximum);
  }

  template<class View>
  forceinline bool
  narrow_bound_product_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                             BoundLocalDomain& z) {
    return View::signed_order ? narrow_bound_signed_product(x,y,z) :
      narrow_bound_unsigned_product(x,y,z);
  }

  /*
   * Bounded actors with an O(width) cube algorithm use two propagation
   * stages. A bound-only event first runs the constant-cost numeric rules and
   * synchronizes each local role once. If that synchronization fixes cube
   * bits, ES_NOFIX_PARTIAL schedules the cube algorithm separately at its
   * honest linear cost. Combined/bit events run both stages locally.
   */
  /// Staged local cube and interval filtering for binary arithmetic.
  template<class View, BoundArithmeticOperation op,
           BoundTerminal terminal=BT_ANY>
  class BoundArithmetic : public TernaryPropagator<View,PC_WORD_DOM> {
  public:
    static bool numeric_regime(View x, View y) {
      static_assert(View::supports_bounds,
                    "bounded arithmetic requires a bounded Word view");
      static_assert((terminal == BT_ANY) || !View::signed_order,
                    "terminal carry/borrow is an unsigned Word property");
      static_assert((terminal == BT_ANY) || (op != BA_MULT),
                    "multiplication has no terminal carry actor");
      if (terminal != BT_ANY)
        return true;
      const WordValue mask=width_mask(x.width());
      if (op == BA_ADD) {
        if (!View::signed_order)
          return x.rank_maximum() <= mask-y.rank_maximum();
        const WordValue sign=sign_bit(x.width());
        return add_signed_ranks(x.rank_minimum(),y.rank_minimum(),
          sign,mask).is_valid &&
          add_signed_ranks(x.rank_maximum(),y.rank_maximum(),sign,mask).is_valid;
      }
      if (op == BA_SUB) {
        if (!View::signed_order)
          return x.rank_minimum() >= y.rank_maximum();
        const WordValue sign=sign_bit(x.width());
        return subtract_signed_ranks(x.rank_minimum(),y.rank_maximum(),
          sign,mask).is_valid &&
          subtract_signed_ranks(x.rank_maximum(),y.rank_minimum(),
            sign,mask).is_valid;
      }
      if (!View::signed_order)
        return (x.rank_maximum() == 0U) ||
          (y.rank_maximum() <= mask/x.rank_maximum());
      const WordValue sign=sign_bit(x.width());
      return multiply_signed_ranks(x.rank_minimum(),y.rank_minimum(),
        sign,mask).is_valid &&
        multiply_signed_ranks(x.rank_minimum(),y.rank_maximum(),
          sign,mask).is_valid &&
        multiply_signed_ranks(x.rank_maximum(),y.rank_minimum(),
          sign,mask).is_valid &&
        multiply_signed_ranks(x.rank_maximum(),y.rank_maximum(),sign,mask).is_valid;
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::ternary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        const BoundFilterResult bounds=narrow_bounds(home,x0,x1,x2);
        GECODE_ES_CHECK(bounds.status);
        const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned();
        if (is_assigned)
          return home.ES_SUBSUMED(*this);
        if (bounds.needs_cube)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      GECODE_ES_CHECK(narrow(home,x0,x1,x2,true));
      const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned();
      return is_assigned ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z) {
      GECODE_ES_CHECK(narrow(home,x,y,z,true));
      const bool is_assigned=x.assigned() && y.assigned() && z.assigned();
      if (!is_assigned)
        (void) new (home) BoundArithmetic(home,x,y,z);
      return ES_OK;
    }
  protected:
    using TernaryPropagator<View,PC_WORD_DOM>::x0;
    using TernaryPropagator<View,PC_WORD_DOM>::x1;
    using TernaryPropagator<View,PC_WORD_DOM>::x2;
    BoundArithmetic(Home home, View x, View y, View z)
      : TernaryPropagator<View,PC_WORD_DOM>(home,x,y,z) {}
    BoundArithmetic(Space& home, BoundArithmetic& p)
      : TernaryPropagator<View,PC_WORD_DOM>(home,p) {}
    /// Apply the selected interval relation to already bound local roles.
    static bool narrow_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                              BoundLocalDomain& z) {
      if (terminal != BT_ANY)
        return narrow_bound_terminal_ranges<op>(x,y,z,terminal);
      if (op == BA_ADD) return narrow_bound_add_ranges<View>(x,y,z);
      if (op == BA_SUB) return narrow_bound_sub_ranges<View>(x,y,z);
      return narrow_bound_product_ranges<View>(x,y,z);
    }
    static BoundFilterResult narrow_bounds(Home home, View x, View y, View z) {
      const View input[3]={x,y,z};
      BoundLocalPass<View,3> pass(input);
      const BoundCubeSnapshot<3> initial=pass.snapshot_bits();
      for (;;) {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        pass.defer_synchronization();
        const bool is_consistent=narrow_ranges(
          pass.domain(0),pass.domain(1),pass.domain(2));
        if (!is_consistent) return BoundFilterResult{ES_FAILED,false};
        if (!pass.synchronize_changed(previous))
          return BoundFilterResult{ES_FAILED,false};
        if (pass.is_unchanged(previous))
          break;
      }
      const bool has_new_bits=pass.has_new_bits(initial);
      return BoundFilterResult{pass.publish(home),has_new_bits};
    }
    static ExecStatus narrow(Home home, View x, View y, View z,
                             bool needs_cube) {
      const View input[3]={x,y,z};
      BoundLocalPass<View,3> pass(input);
      for (;;) {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        pass.defer_synchronization();
        const bool needs_add=needs_cube && (op == BA_ADD);
        if (needs_add) {
          GECODE_ES_CHECK(narrow_add(
            home,pass.view(0),pass.view(1),pass.view(2),terminal).status);
        }
        const bool needs_subtract=needs_cube && (op == BA_SUB);
        if (needs_subtract) {
          GECODE_ES_CHECK(narrow_subtract(
            home,pass.view(0),pass.view(1),pass.view(2),terminal).status);
        }
        const bool needs_multiply=needs_cube && (op == BA_MULT);
        if (needs_multiply)
          GECODE_ES_CHECK(narrow_product(
            home,pass.view(0),pass.view(1),pass.view(2)));
        const BoundCubeSnapshot<3> before_ranges=pass.snapshot_bits();
        const bool is_consistent=narrow_ranges(
          pass.domain(0),pass.domain(1),pass.domain(2));
        if (!is_consistent) return ES_FAILED;
        if (!pass.synchronize_changed(previous)) return ES_FAILED;
        needs_cube=pass.has_new_bits(before_ranges);
        if (pass.is_unchanged(previous))
          break;
      }
      return pass.publish(home);
    }
  };

}}}

#endif
