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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_TERMINAL_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_TERMINAL_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline WordValue
  add_saturating_ranks(WordValue x, WordValue y, WordValue mask) {
    return (x > mask-y) ? mask : x+y;
  }

  /// First operand projection and the second operand's frozen lower bound.
  struct BoundClearOperandsResult {
    bool has_support;
    WordRankInterval x;
    WordValue y_minimum;
  };

  /// Both inverse operand intervals, calculated before either operand tell.
  struct BoundOperandsResult {
    bool has_support;
    WordRankInterval x, y;
  };

  forceinline WordRankInterval
  compute_add_clear_interval(WordRankInterval x, WordRankInterval y,
                              WordValue mask) {
    return {x.minimum+y.minimum,
            add_saturating_ranks(x.maximum,y.maximum,mask)};
  }

  forceinline BoundClearOperandsResult
  project_add_clear_operands(WordRankInterval x, WordRankInterval y,
                             WordRankInterval z) {
    const bool has_unsupported_result=(z.maximum < y.minimum) ||
      (z.maximum < x.minimum);
    if (has_unsupported_result)
      return {false,{0,0},0};
    const WordValue xmin=(z.minimum > y.maximum) ? z.minimum-y.maximum : 0U;
    const WordValue ymin=(z.minimum > x.maximum) ? z.minimum-x.maximum : 0U;
    return {true,{std::max(x.minimum,xmin),
                  std::min(x.maximum,z.maximum-y.minimum)},
            std::max(y.minimum,ymin)};
  }

  forceinline WordRankInterval
  project_add_clear_second_operand(WordRankInterval y, WordValue x_minimum,
                                    WordValue y_minimum, WordValue z_maximum) {
    return {y_minimum,std::min(y.maximum,z_maximum-x_minimum)};
  }

  /// Restrict nonwrapping addition, retaining live result reads for aliases.
  forceinline bool
  narrow_bound_add_clear(BoundLocalDomain& x, BoundLocalDomain& y,
                         BoundLocalDomain& z, WordValue mask) {
    const bool is_inconsistent=
      !x.intersect_range(x.minimum,std::min(x.maximum,mask-y.minimum)) ||
      !y.intersect_range(y.minimum,std::min(y.maximum,mask-x.minimum));
    if (is_inconsistent)
      return false;
    const WordRankInterval old_x=snapshot_bound_interval(x);
    const WordRankInterval old_y=snapshot_bound_interval(y);
    const WordRankInterval forward=compute_add_clear_interval(old_x,old_y,mask);
    if (!z.intersect_range(forward.minimum,forward.maximum))
      return false;
    const BoundClearOperandsResult inverse=project_add_clear_operands(
      old_x,old_y,snapshot_bound_interval(z));
    if (!inverse.has_support)
      return false;
    if (!x.intersect_range(inverse.x.minimum,inverse.x.maximum))
      return false;
    // An aliased x tell can tighten z; only y's upper bound reads it again.
    const WordRankInterval second=project_add_clear_second_operand(
      old_y,old_x.minimum,inverse.y_minimum,z.maximum);
    return y.intersect_range(second.minimum,second.maximum);
  }

  forceinline WordRankInterval
  compute_add_set_interval(WordRankInterval x, WordRankInterval y,
                            WordValue mask) {
    const WordValue minimum=(x.minimum > mask-y.minimum) ?
      x.minimum-(mask-y.minimum)-1U : 0U;
    return {minimum,x.maximum-(mask-y.maximum)-1U};
  }

  forceinline BoundOperandsResult
  project_add_set_operands(WordRankInterval x, WordRankInterval y,
                           WordRankInterval z, WordValue mask) {
    const WordValue xbase=mask-y.maximum+1U;
    const WordValue ybase=mask-x.maximum+1U;
    const bool has_unsupported_result=(z.minimum > mask-xbase) ||
      (z.minimum > mask-ybase);
    if (has_unsupported_result)
      return {false,{0,0},{0,0}};
    const WordValue xmin=xbase+z.minimum, ymin=ybase+z.minimum;
    const WordValue xmax=add_saturating_ranks(mask-y.minimum+1U,z.maximum,mask);
    const WordValue ymax=add_saturating_ranks(mask-x.minimum+1U,z.maximum,mask);
    return {true,{std::max(x.minimum,xmin),std::min(x.maximum,xmax)},
                 {std::max(y.minimum,ymin),std::min(y.maximum,ymax)}};
  }

  /// Restrict wrapping addition with inverse intervals frozen before tells.
  forceinline bool
  narrow_bound_add_set(BoundLocalDomain& x, BoundLocalDomain& y,
                       BoundLocalDomain& z, WordValue mask) {
    const bool is_inconsistent=(x.maximum <= mask-y.maximum) ||
      !x.intersect_range(std::max(x.minimum,mask-y.maximum+1U),x.maximum) ||
      !y.intersect_range(std::max(y.minimum,mask-x.maximum+1U),y.maximum);
    if (is_inconsistent)
      return false;
    const WordRankInterval old_x=snapshot_bound_interval(x);
    const WordRankInterval old_y=snapshot_bound_interval(y);
    const WordRankInterval forward=compute_add_set_interval(old_x,old_y,mask);
    if (!z.intersect_range(forward.minimum,forward.maximum))
      return false;
    const BoundOperandsResult inverse=project_add_set_operands(
      old_x,old_y,snapshot_bound_interval(z),mask);
    if (!inverse.has_support)
      return false;
    return x.intersect_range(inverse.x.minimum,inverse.x.maximum) &&
      y.intersect_range(inverse.y.minimum,inverse.y.maximum);
  }

  forceinline WordRankInterval
  compute_sub_clear_interval(WordRankInterval x, WordRankInterval y) {
    const WordValue minimum=(x.minimum > y.maximum) ? x.minimum-y.maximum : 0U;
    return {minimum,x.maximum-y.minimum};
  }

  forceinline BoundClearOperandsResult
  project_sub_clear_operands(WordRankInterval x, WordRankInterval y,
                             WordRankInterval z, WordValue mask) {
    const WordValue xmin=add_saturating_ranks(z.minimum,y.minimum,mask);
    const WordValue xmax=add_saturating_ranks(z.maximum,y.maximum,mask);
    const WordValue ymin=(x.minimum > z.maximum) ? x.minimum-z.maximum : 0U;
    return {true,{std::max(x.minimum,xmin),std::min(x.maximum,xmax)},
            std::max(y.minimum,ymin)};
  }

  forceinline WordRankInterval
  project_sub_clear_second_operand(WordRankInterval y, WordValue x_maximum,
                                    WordValue y_minimum, WordValue z_minimum) {
    return {y_minimum,std::min(y.maximum,x_maximum-z_minimum)};
  }

  /// Restrict nonwrapping subtraction, retaining live result reads for aliases.
  forceinline bool
  narrow_bound_sub_clear(BoundLocalDomain& x, BoundLocalDomain& y,
                         BoundLocalDomain& z, WordValue mask) {
    const bool is_inconsistent=
      !x.intersect_range(std::max(x.minimum,y.minimum),x.maximum) ||
      !y.intersect_range(y.minimum,std::min(y.maximum,x.maximum));
    if (is_inconsistent)
      return false;
    const WordRankInterval old_x=snapshot_bound_interval(x);
    const WordRankInterval old_y=snapshot_bound_interval(y);
    const WordRankInterval forward=compute_sub_clear_interval(old_x,old_y);
    if (!z.intersect_range(forward.minimum,forward.maximum))
      return false;
    const BoundClearOperandsResult inverse=project_sub_clear_operands(
      old_x,old_y,snapshot_bound_interval(z),mask);
    if (!inverse.has_support)
      return false;
    if (!x.intersect_range(inverse.x.minimum,inverse.x.maximum))
      return false;
    // An aliased x tell can tighten z; only y's upper bound reads it again.
    const WordRankInterval second=project_sub_clear_second_operand(
      old_y,old_x.maximum,inverse.y_minimum,z.minimum);
    return y.intersect_range(second.minimum,second.maximum);
  }

  forceinline WordRankInterval
  compute_sub_set_interval(WordRankInterval x, WordRankInterval y,
                            WordValue mask) {
    const WordValue minimum=mask-(y.maximum-x.minimum)+1U;
    const WordValue maximum=(x.maximum < y.minimum) ?
      mask-(y.minimum-x.maximum)+1U : mask;
    return {minimum,maximum};
  }

  forceinline BoundOperandsResult
  project_sub_set_operands(WordRankInterval x, WordRankInterval y,
                           WordRankInterval z, WordValue mask) {
    const WordValue gap_min=mask-z.maximum+1U;
    const WordValue gap_max=mask-z.minimum+1U;
    const bool has_unsupported_gap=(y.maximum < gap_min) ||
      (x.minimum > mask-gap_min);
    if (has_unsupported_gap)
      return {false,{0,0},{0,0}};
    const WordValue xmin=(y.minimum > gap_max) ? y.minimum-gap_max : 0U;
    const WordValue xmax=y.maximum-gap_min, ymin=x.minimum+gap_min;
    const WordValue ymax=add_saturating_ranks(x.maximum,gap_max,mask);
    return {true,{std::max(x.minimum,xmin),std::min(x.maximum,xmax)},
                 {std::max(y.minimum,ymin),std::min(y.maximum,ymax)}};
  }

  /// Restrict wrapping subtraction with inverse intervals frozen before tells.
  forceinline bool
  narrow_bound_sub_set(BoundLocalDomain& x, BoundLocalDomain& y,
                       BoundLocalDomain& z, WordValue mask) {
    const bool is_inconsistent=(x.minimum >= y.maximum) ||
      !x.intersect_range(x.minimum,std::min(x.maximum,y.maximum-1U)) ||
      !y.intersect_range(std::max(y.minimum,x.minimum+1U),y.maximum);
    if (is_inconsistent)
      return false;
    const WordRankInterval old_x=snapshot_bound_interval(x);
    const WordRankInterval old_y=snapshot_bound_interval(y);
    const WordRankInterval forward=compute_sub_set_interval(old_x,old_y,mask);
    if (!z.intersect_range(forward.minimum,forward.maximum))
      return false;
    const BoundOperandsResult inverse=project_sub_set_operands(
      old_x,old_y,snapshot_bound_interval(z),mask);
    if (!inverse.has_support)
      return false;
    return x.intersect_range(inverse.x.minimum,inverse.x.maximum) &&
      y.intersect_range(inverse.y.minimum,inverse.y.maximum);
  }

  template<BoundArithmeticOperation op>
  forceinline bool
  narrow_bound_terminal_ranges(BoundLocalDomain& x, BoundLocalDomain& y,
                               BoundLocalDomain& z,
                               unsigned int terminal) {
    const WordValue mask=width_mask(x.width);
    if (op == BA_ADD)
      return (terminal == BT_CLEAR) ?
        narrow_bound_add_clear(x,y,z,mask) :
        narrow_bound_add_set(x,y,z,mask);
    return (terminal == BT_CLEAR) ?
      narrow_bound_sub_clear(x,y,z,mask) :
      narrow_bound_sub_set(x,y,z,mask);
  }

}}}

#endif
