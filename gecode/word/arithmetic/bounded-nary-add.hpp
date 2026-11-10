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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_NARY_ADD_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_NARY_ADD_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-rank.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Optional ranked minima and maxima for consecutive partial sums.
  struct BoundPartialSums {
    WordValue* minimum;
    WordValue* maximum;
    bool* has_minimum;
    bool* has_maximum;
  };

  /// Prefix and suffix ranks for one non-wrapping n-ary sum.
  struct BoundNarySumRanks {
    BoundPartialSums prefix, suffix;
    int size;
    WordValue mask, sign;
  };

  /// Independently available endpoints of one checked partial sum.
  struct BoundSumEndpoints {
    BoundRankResult minimum, maximum;
  };

  /// Unsigned contradiction or the next optional partial-sum endpoints.
  struct BoundSumExtensionResult {
    bool is_consistent;
    BoundSumEndpoints endpoints;
  };

  /// Completed sum ranks backed by the caller's Region, or a contradiction.
  struct BoundNarySumResult {
    bool is_consistent;
    BoundNarySumRanks sums;
  };

  template<class View>
  forceinline BoundSumEndpoints
  read_bound_partial_sum(const BoundPartialSums& sums, int i) {
    const bool has_minimum=!View::signed_order || sums.has_minimum[i];
    const bool has_maximum=!View::signed_order || sums.has_maximum[i];
    return {{has_minimum,has_minimum ? sums.minimum[i] : 0},
            {has_maximum,has_maximum ? sums.maximum[i] : 0}};
  }

  template<class View>
  forceinline void
  write_bound_partial_sum(BoundPartialSums& sums, int i,
                          BoundSumEndpoints endpoints) {
    if (View::signed_order) {
      sums.has_minimum[i]=endpoints.minimum.is_valid;
      sums.has_maximum[i]=endpoints.maximum.is_valid;
    }
    if (endpoints.minimum.is_valid) sums.minimum[i]=endpoints.minimum.value;
    if (endpoints.maximum.is_valid) sums.maximum[i]=endpoints.maximum.value;
  }

  /// Extend one optional interval without reading an unavailable endpoint.
  template<class View>
  forceinline BoundSumExtensionResult
  extend_bound_partial_sum(BoundSumEndpoints current, WordRankInterval operand,
                           WordValue sign, WordValue mask) {
    if (!View::signed_order) {
      const bool has_overflow=(operand.minimum > mask-current.minimum.value) ||
        (operand.maximum > mask-current.maximum.value);
      if (has_overflow) return {false,{}};
      return {true,{{true,current.minimum.value+operand.minimum},
                    {true,current.maximum.value+operand.maximum}}};
    }
    const BoundRankResult minimum=current.minimum.is_valid ?
      add_signed_ranks(current.minimum.value,operand.minimum,sign,mask) :
      BoundRankResult{false,0};
    const BoundRankResult maximum=current.maximum.is_valid ?
      add_signed_ranks(current.maximum.value,operand.maximum,sign,mask) :
      BoundRankResult{false,0};
    return {true,{minimum,maximum}};
  }

  template<class View>
  forceinline bool
  fill_bound_prefix_sums(BoundLocalDomain** input, int n,
                        BoundPartialSums& prefix,
                        WordValue sign, WordValue mask) {
    for (int i=0; i<n; i++) {
      const BoundSumExtensionResult next=extend_bound_partial_sum<View>(
        read_bound_partial_sum<View>(prefix,i),
        snapshot_bound_interval(*input[i]),sign,mask);
      if (!next.is_consistent) return false;
      write_bound_partial_sum<View>(prefix,i+1,next.endpoints);
    }
    return true;
  }

  template<class View>
  forceinline bool
  fill_bound_suffix_sums(BoundLocalDomain** input, int n,
                        BoundPartialSums& suffix,
                        WordValue sign, WordValue mask) {
    for (int i=n; i-- > 0;) {
      const BoundSumExtensionResult next=extend_bound_partial_sum<View>(
        read_bound_partial_sum<View>(suffix,i+1),
        snapshot_bound_interval(*input[i]),sign,mask);
      if (!next.is_consistent) return false;
      write_bound_partial_sum<View>(suffix,i,next.endpoints);
    }
    return true;
  }

  template<class View>
  forceinline BoundPartialSums
  allocate_bound_partial_sums(Region& region, int n) {
    return {region.alloc<WordValue>(n+1),region.alloc<WordValue>(n+1),
            View::signed_order ? region.alloc<bool>(n+1) : nullptr,
            View::signed_order ? region.alloc<bool>(n+1) : nullptr};
  }

  /// Prepare both scans before any result or operand domain is narrowed.
  template<class View>
  forceinline BoundNarySumResult
  prepare_bound_nary_sums(Region& region, BoundLocalDomain** input, int n,
                         const BoundLocalDomain& result, WordValue constant) {
    BoundPartialSums prefix=allocate_bound_partial_sums<View>(region,n);
    BoundPartialSums suffix=allocate_bound_partial_sums<View>(region,n);
    const WordValue mask=width_mask(result.width);
    const WordValue sign=View::signed_order ? sign_bit(result.width) : 0;
    const WordValue initial=Word::rank(result.kind,result.width,constant);
    write_bound_partial_sum<View>(prefix,0,{{true,initial},{true,initial}});
    write_bound_partial_sum<View>(suffix,n,{{true,sign},{true,sign}});
    if (!fill_bound_prefix_sums<View>(input,n,prefix,sign,mask))
      return {false,{}};
    if (!fill_bound_suffix_sums<View>(input,n,suffix,sign,mask))
      return {false,{}};
    return {true,{prefix,suffix,n,mask,sign}};
  }

  /// Frozen prefix and suffix endpoints surrounding one sum operand.
  struct BoundSumNeighbors {
    WordValue prefix_minimum, prefix_maximum;
    WordValue suffix_minimum, suffix_maximum;
    bool has_minimum, has_maximum;
  };

  /// An operand interval, an unavailable deduction, or a contradiction.
  struct BoundIntervalFilterResult {
    enum class Status { INTERVAL, UNAVAILABLE, CONTRADICTION };
    Status status;
    WordRankInterval interval;
  };

  template<class View>
  forceinline BoundSumNeighbors
  select_bound_sum_neighbors(const BoundNarySumRanks& sums, int i) {
    const bool has_minimum=!View::signed_order ||
      (sums.prefix.has_minimum[i] && sums.suffix.has_minimum[i+1]);
    const bool has_maximum=!View::signed_order ||
      (sums.prefix.has_maximum[i] && sums.suffix.has_maximum[i+1]);
    return BoundSumNeighbors{
      has_minimum ? sums.prefix.minimum[i] : 0,
      has_maximum ? sums.prefix.maximum[i] : 0,
      has_minimum ? sums.suffix.minimum[i+1] : 0,
      has_maximum ? sums.suffix.maximum[i+1] : 0,
      has_minimum,has_maximum};
  }

  /// Merge frozen neighboring sums before applying inverse result bounds.
  template<class View>
  forceinline BoundIntervalFilterResult
  merge_bound_sum_neighbors(const BoundSumNeighbors& neighbors,
                            WordValue sign, WordValue mask) {
    if (!View::signed_order) {
      const bool has_overflow=
        (neighbors.suffix_minimum > mask-neighbors.prefix_minimum) ||
        (neighbors.suffix_maximum > mask-neighbors.prefix_maximum);
      if (has_overflow)
        return {BoundIntervalFilterResult::Status::CONTRADICTION,{0,0}};
      return {BoundIntervalFilterResult::Status::INTERVAL,
        {neighbors.prefix_minimum+neighbors.suffix_minimum,
         neighbors.prefix_maximum+neighbors.suffix_maximum}};
    }
    const BoundRankResult minimum=neighbors.has_minimum ?
      add_signed_ranks(neighbors.prefix_minimum,neighbors.suffix_minimum,
                       sign,mask) : BoundRankResult{false,0};
    const BoundRankResult maximum=neighbors.has_maximum ?
      add_signed_ranks(neighbors.prefix_maximum,neighbors.suffix_maximum,
                       sign,mask) : BoundRankResult{false,0};
    const bool has_bounds=minimum.is_valid && maximum.is_valid;
    if (!has_bounds)
      return {BoundIntervalFilterResult::Status::UNAVAILABLE,{0,0}};
    return {BoundIntervalFilterResult::Status::INTERVAL,{minimum.value,maximum.value}};
  }

  template<class View>
  forceinline BoundIntervalFilterResult
  project_bound_sum_operand(BoundIntervalFilterResult others,
                            WordRankInterval result,
                            WordValue sign, WordValue mask) {
    if (others.status != BoundIntervalFilterResult::Status::INTERVAL) return others;
    if (!View::signed_order) {
      if (result.maximum < others.interval.minimum)
        return {BoundIntervalFilterResult::Status::CONTRADICTION,{0,0}};
      const WordValue minimum=(result.minimum >= others.interval.maximum) ?
        result.minimum-others.interval.maximum : 0;
      return {BoundIntervalFilterResult::Status::INTERVAL,
        {minimum,result.maximum-others.interval.minimum}};
    }
    const BoundRankResult minimum=subtract_signed_ranks(
      result.minimum,others.interval.maximum,sign,mask);
    const BoundRankResult maximum=subtract_signed_ranks(
      result.maximum,others.interval.minimum,sign,mask);
    return {BoundIntervalFilterResult::Status::INTERVAL,
      {minimum.is_valid ? minimum.value : 0,
       maximum.is_valid ? maximum.value : mask}};
  }

  /// Deduce one operand's interval without mutating any local domain.
  template<class View>
  forceinline BoundIntervalFilterResult
  compute_bound_operand_interval(const BoundSumNeighbors& neighbors,
                                 WordRankInterval result,
                                 WordValue sign, WordValue mask) {
    const BoundIntervalFilterResult others=
      merge_bound_sum_neighbors<View>(neighbors,sign,mask);
    return project_bound_sum_operand<View>(others,result,sign,mask);
  }

  /// Intersect operands in occurrence order using frozen sums and live results.
  template<class View>
  forceinline bool
  narrow_bound_sum_inputs(BoundLocalDomain** input, BoundLocalDomain& result,
                          const BoundNarySumRanks& sums) {
    for (int i=0; i<sums.size; i++) {
      const BoundSumNeighbors neighbors=select_bound_sum_neighbors<View>(sums,i);
      const BoundIntervalFilterResult deduction=compute_bound_operand_interval<View>(
        neighbors,WordRankInterval{result.minimum,result.maximum},
        sums.sign,sums.mask);
      if (deduction.status == BoundIntervalFilterResult::Status::CONTRADICTION)
        return false;
      if (deduction.status == BoundIntervalFilterResult::Status::UNAVAILABLE)
        continue;
      if (!input[i]->intersect_range(
            std::max(input[i]->minimum,deduction.interval.minimum),
            std::min(input[i]->maximum,deduction.interval.maximum)))
        return false;
    }
    return true;
  }

  /// Filter the result and operands using prefix and suffix sum bounds.
  template<class View>
  forceinline bool
  narrow_bound_nary_add_ranges(BoundLocalDomain** input, int n,
                               BoundLocalDomain& result, WordValue constant) {
    Region region;
    const BoundNarySumResult prepared=prepare_bound_nary_sums<View>(
      region,input,n,result,constant);
    if (!prepared.is_consistent) return false;
    const BoundNarySumRanks& sums=prepared.sums;
    const bool has_result_bounds=!View::signed_order ||
      (sums.prefix.has_minimum[n] && sums.prefix.has_maximum[n]);
    const bool has_empty_result=has_result_bounds &&
      !result.intersect_range(sums.prefix.minimum[n],sums.prefix.maximum[n]);
    if (has_empty_result) return false;
    return narrow_bound_sum_inputs<View>(input,result,sums);
  }

  /** \brief Cancel one result occurrence before subscribing or filtering
   *
   * Return ES_OK for a solved residual, ES_FAILED for a contradiction, and
   * ES_FIX when posting must continue. Other input occurrences keep their order.
   */
  template<class View>
  forceinline ExecStatus
  cancel_nary_add_result(Home home, ViewArray<View>& input, View& result,
                         WordValue constant) {
    int occurrence=0;
    for (;;) {
      const bool has_unmatched_input=(occurrence < input.size()) &&
        (input[occurrence].varimp() != result.varimp());
      if (!has_unmatched_input) break;
      occurrence++;
    }
    if (occurrence == input.size())
      return ES_FIX;
    for (int i=occurrence+1; i<input.size(); i++)
      input[i-1]=input[i];
    input.size(input.size()-1);
    const WordValue mask=result.mask();
    if (input.size() == 0)
      return (constant & mask) == 0 ? ES_OK : ES_FAILED;
    if (input.size() == 1) {
      GECODE_ME_CHECK(input[0].eq(home,(WordValue(0)-constant)&mask));
      return ES_OK;
    }
    const WordDomainType kind=View::signed_order ? WDT_SIGNED : WDT_UNSIGNED;
    result=View(WordVar(home,result.width(),kind,0,0));
    return ES_FIX;
  }

  /// Test nonwrapping eligibility after result cancellation.
  template<class View>
  forceinline bool
  can_bound_nary_add(const ViewArray<View>& x, View y, WordValue c,
                     bool skip_assigned=false) {
    const WordValue mask=y.mask(), sign=View::signed_order ? sign_bit(y.width()) : 0;
    WordValue minimum=c^sign, maximum=minimum;
    for (int i=0; i<x.size(); i++) {
      if (skip_assigned && x[i].assigned()) continue;
      if (View::signed_order) {
        const BoundRankResult next_minimum=
          add_signed_ranks(minimum,x[i].rank_minimum(),sign,mask);
        if (!next_minimum.is_valid) return false;
        const BoundRankResult next_maximum=
          add_signed_ranks(maximum,x[i].rank_maximum(),sign,mask);
        if (!next_maximum.is_valid) return false;
        minimum=next_minimum.value; maximum=next_maximum.value;
      } else {
        if (x[i].rank_maximum() > mask-maximum) return false;
        maximum+=x[i].rank_maximum();
      }
    }
    return true;
  }

  /// Signed regrouping must retain the nonwrapping prefix guarantee.
  template<class View>
  forceinline bool
  compact_bound_nary_add(ViewArray<View>& x, View y, WordValue& constant) {
    if (View::signed_order) {
      WordValue folded=constant;
      bool has_assigned=false;
      for (int i=0; i<x.size(); i++)
        if (x[i].assigned()) {
          folded+=x[i].val();
          has_assigned=true;
        }
      if (!has_assigned ||
          !can_bound_nary_add(x,y,folded & y.mask(),true))
        return false;
    }
    return compact_nary_add(x,constant,y.mask());
  }

  template<class View>
  forceinline
  BoundNaryAdd<View>::BoundNaryAdd(Home home, ViewArray<View>& x0, View y0,
                                   WordValue c, bool has_aliases)
    : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(home,x0,y0),
      constant(c), has_aliases(has_aliases) {}

  template<class View>
  forceinline
  BoundNaryAdd<View>::BoundNaryAdd(Space& home, BoundNaryAdd& p)
    : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(home,p),
      constant(p.constant), has_aliases(p.has_aliases) {}

  template<class View>
  Actor*
  BoundNaryAdd<View>::copy(Space& home) {
    return new (home) BoundNaryAdd(home,*this);
  }

  template<class View>
  size_t
  BoundNaryAdd<View>::dispose(Space& home) {
    (void) MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  PropCost
  BoundNaryAdd<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_WORD_BND)
      return has_aliases ? PropCost::quadratic(PropCost::LO,x.size()+1) :
        PropCost::linear(PropCost::LO,static_cast<unsigned int>(x.size()+1));
    if (has_aliases)
      return PropCost::quadratic(PropCost::HI,x.size()+1);
    return PropCost::linear(PropCost::HI,
                            static_cast<unsigned int>(x.size())*y.width());
  }

  /// Preserve the caller's alias policy and choose first occurrence indices.
  template<class View>
  forceinline int*
  make_bound_nary_representatives(Region& region,
                                 const ViewArray<View>& input, View output,
                                 bool has_aliases) {
    const int n=input.size();
    int* representative=region.alloc<int>(n+1);
    for (int i=0; i<n+1; i++) {
      const View current=(i<n) ? input[i] : output;
      assert((i == n) || (current.varimp() != output.varimp()));
      representative[i]=i;
      if (!has_aliases) continue;
      for (int j=0; j<i; j++) {
        const View previous=input[j];
        if (current.varimp() == previous.varimp()) {
          representative[i]=representative[j];
          break;
        }
      }
    }
    return representative;
  }

  /** \brief Region-backed local domains for one n-ary addition call
   * Input occurrences retain their order and point at the first alias record.
   * The result is distinct after posting cancels its first input occurrence.
   */
  class BoundNaryDomains {
  public:
    /// Progress of a pass and cube work introduced after the cube checkpoint
    struct LocalPass {
      PassProgress progress;
      bool needs_cube;
    };

    template<class View>
    BoundNaryDomains(Region& region, const ViewArray<View>& input, View output,
                    bool has_aliases)
      : inputs(), result(), roles(nullptr), n(input.size()),
        domains(region.alloc<BoundLocalDomain>(n+1)),
        representative(make_bound_nary_representatives(
          region,input,output,has_aliases)) {
      inputs=ViewArray<BoundLocalView>(region,n);
      for (int i=0; i<n+1; i++) {
        const View current=(i<n) ? input[i] : output;
        if (representative[i] == i)
          domains[i]=snapshot_bound_domain(current);
        if (i<n)
          inputs[i]=BoundLocalView(domains[representative[i]]);
      }
      result=BoundLocalView(domains[representative[n]]);
      roles=region.alloc<BoundLocalDomain*>(n);
      old=region.alloc<BoundLocalDomain>(n+1);
      deduced=region.alloc<WordRankInterval>(n+1);
      before_lo=region.alloc<WordValue>(n+1);
      before_hi=region.alloc<WordValue>(n+1);
      for (int i=0; i<n; i++)
        roles[i]=&domains[representative[i]];
    }

    /// Run one complete cube/range pass and classify it (see PassProgress)
    template<class View>
    LocalPass narrow_pass(Home home, WordValue constant, bool needs_cube) {
      begin_pass();
      if (needs_cube) {
        const ExecStatus status=NaryAdd::narrow(home,inputs,result,constant);
        if (status < ES_OK) return LocalPass{PassProgress::FAILED,false};
      }
      remember_cubes();
      if (!narrow_bound_nary_add_ranges<View>(
            roles,n,domains[representative[n]],constant))
        return LocalPass{PassProgress::FAILED,false};
      remember_deductions();
      if (!synchronize()) return LocalPass{PassProgress::FAILED,false};
      return inspect_changes();
    }

    /** \brief Tell each representative through its original occurrence's view
     *
     * The result is ES_NOFIX when a value set closes beyond its record.
     */
    template<class View>
    ExecStatus publish(Home home, const ViewArray<View>& input, View output) {
      ExecStatus status=ES_FIX;
      for (int i=0; i<n; i++)
        if (representative[i] == i) {
          status=combine_publication(
            status,publish_bound_domain(home,input[i],domains[i]));
          GECODE_ES_CHECK(status);
        }
      if (representative[n] == n)
        status=combine_publication(
          status,publish_bound_domain(home,output,domains[n]));
      return status;
    }
  private:
    void begin_pass(void) {
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          old[i]=domains[i];
          domains[i].deferred=true;
        }
    }

    void remember_cubes(void) {
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          before_lo[i]=domains[i].lo;
          before_hi[i]=domains[i].hi;
        }
    }

    void remember_deductions(void) {
      for (int i=0; i<n+1; i++)
        if (representative[i] == i)
          deduced[i]=snapshot_bound_interval(domains[i]);
    }

    bool synchronize(void) {
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          domains[i].deferred=false;
          // Passes start synchronized, so unchanged roles need no new closure.
          const bool has_failed=!(domains[i] == old[i]) && !domains[i].synchronize();
          if (has_failed)
            return false;
        }
      return true;
    }

    /// Classify the pass and report new bits since the optional cube work
    LocalPass inspect_changes(void) const {
      bool has_changed=false, has_new_bits=false, has_jump=false;
      bool needs_cube=false;
      for (int i=0; i<n+1; i++)
        if (representative[i] == i) {
          has_changed |= !(domains[i] == old[i]);
          has_new_bits |= (domains[i].lo != old[i].lo) ||
            (domains[i].hi != old[i].hi);
          has_jump |= (domains[i].minimum != deduced[i].minimum) ||
            (domains[i].maximum != deduced[i].maximum);
          needs_cube |= (domains[i].lo != before_lo[i]) ||
            (domains[i].hi != before_hi[i]);
        }
      return LocalPass{classify_pass(has_changed,has_new_bits,has_jump),
                       needs_cube};
    }

    ViewArray<BoundLocalView> inputs;
    BoundLocalView result;
    BoundLocalDomain** roles;
    int n;
    BoundLocalDomain* domains;
    int* representative;
    BoundLocalDomain* old;
    WordRankInterval* deduced;
    WordValue* before_lo;
    WordValue* before_hi;
  };

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::narrow(Home home, ViewArray<View>& input, View result,
                            WordValue c, bool has_aliases) {
    return repeat_until_closed([&](void) {
      return narrow_once(home,input,result,c,true,has_aliases);
    });
  }

  /// Close the permitted stages locally; bounds calls never enter cube work.
  template<class View>
  PassResult
  BoundNaryAdd<View>::narrow_once(Home home, ViewArray<View>& input,
                                  View result, WordValue c, bool can_run_cube,
                                  bool has_aliases) {
    Region region;
    BoundNaryDomains local(region,input,result,has_aliases);
    bool needs_cube=can_run_cube;
    PassProgress progress;
    // A closure jump stops the loop: a bounds-only call hands over to the cube
    // stage, and the expensive full stage lets cheaper propagators run first.
    do {
      const BoundNaryDomains::LocalPass pass=local.narrow_pass<View>(
        home,c,needs_cube);
      progress=pass.progress;
      if (progress == PassProgress::FAILED) return PassResult::FAILED;
      needs_cube=can_run_cube && pass.needs_cube;
    } while (progress == PassProgress::CHANGED);
    return pass_result(local.publish(home,input,result),progress);
  }

  /// Close only ranges and report new bits for a separately scheduled stage.
  template<class View>
  BoundFilterResult
  BoundNaryAdd<View>::narrow_bounds(Home home, ViewArray<View>& input,
                                    View result, WordValue c, bool has_aliases) {
    WordValue before_lo=result.lo(), before_hi=result.hi();
    Region region;
    WordValue* lo=region.alloc<WordValue>(input.size());
    WordValue* hi=region.alloc<WordValue>(input.size());
    for (int i=0; i<input.size(); i++) {
      lo[i]=input[i].lo(); hi[i]=input[i].hi();
    }
    const PassResult pass=narrow_once(home,input,result,c,false,has_aliases);
    if (pass == PassResult::FAILED) return BoundFilterResult{ES_FAILED,false};
    bool has_new_bits=(before_lo != result.lo()) || (before_hi != result.hi());
    for (int i=0; i<input.size(); i++)
      has_new_bits |= (lo[i] != input[i].lo()) || (hi[i] != input[i].hi());
    return finish_bound_stage(pass,has_new_bits,[&](void) {
      return narrow(home,input,result,c,has_aliases);
    });
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::propagate(Space& home, const ModEventDelta& med) {
    const bool compacted=compact_bound_nary_add(x,y,constant);
    if (View::me(med) == ME_WORD_BND) {
      bool needs_cube=compacted;
      do {
        const BoundFilterResult bounds=narrow_bounds(home,x,y,constant,has_aliases);
        GECODE_ES_CHECK(bounds.status);
        needs_cube |= bounds.needs_cube;
        if (y.assigned() && x.assigned()) return home.ES_SUBSUMED(*this);
      } while (compact_bound_nary_add(x,y,constant));
      return needs_cube ?
        home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS)) :
        ES_FIX;
    }
    ExecStatus status;
    do {
      status=narrow(home,x,y,constant,has_aliases);
      GECODE_ES_CHECK(status);
      if (y.assigned() && x.assigned()) return home.ES_SUBSUMED(*this);
    } while (compact_bound_nary_add(x,y,constant));
    return status;
  }

  template<class View>
  ExecStatus
  BoundNaryAdd<View>::post(Home home, ViewArray<View>& input, View result,
                           WordValue c) {
    const ExecStatus es=cancel_nary_add_result(home,input,result,c);
    if (es != ES_FIX) return es;
    if (!can_bound_nary_add(input,result,c)) {
      ViewArray<WordView> modular(home,input.size());
      for (int i=0; i<input.size(); i++)
        modular[i]=WordView(input[i].varimp());
      return NaryAdd::post(home,modular,WordView(result.varimp()),c);
    }
    const bool has_aliases=has_nary_add_aliases(input,result);
    compact_bound_nary_add(input,result,c);
    ExecStatus status;
    do {
      status=narrow(home,input,result,c,has_aliases);
      GECODE_ES_CHECK(status);
      if (result.assigned() && input.assigned()) return ES_OK;
    } while (compact_bound_nary_add(input,result,c));
    schedule_unless_fixpoint(
      home,*new (home) BoundNaryAdd(home,input,result,c,has_aliases),status);
    return ES_OK;
  }

}}}

#endif
