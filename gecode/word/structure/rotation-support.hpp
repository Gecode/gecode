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

  /// Known-one and possible-one masks for a rotation role or count class.
  struct RotationBits {
    WordValue lo, hi;
  };

  forceinline bool
  is_empty(RotationBits bits) {
    return (bits.lo & ~bits.hi) != 0;
  }

  forceinline RotationBits
  merge_bits(RotationBits left, RotationBits right) {
    if (is_empty(left)) return right;
    if (is_empty(right)) return left;
    return {left.lo & right.lo, left.hi | right.hi};
  }

  /// Current cube projections for the three independent rotation roles.
  struct RotationDomains {
    RotationBits input, count, result;
  };

  /// Native views for independent rotation roles.
  struct RotationViews {
    WordView input, count, result;
  };

  /// Count hulls indexed by unsigned residue modulo the word width.
  struct RotationClasses {
    // The table belongs to the Region and expires at the next compute call.
    const RotationBits* hulls;
    WordValue reachable;
  };

  /// Computes count hulls in O(width squared) time and O(width) region storage.
  class RotationCountClasses {
  public:
    RotationCountClasses(Region& region, unsigned int width);
    RotationClasses compute(RotationBits count);
  private:
    unsigned int width;
    RotationBits* current;
    RotationBits* next;
    void extend_bit(RotationBits count, WordValue bit, unsigned int weight);
  };

  forceinline
  RotationCountClasses::RotationCountClasses(Region& region, unsigned int w)
    : width(w), current(region.alloc<RotationBits>(w)),
      next(region.alloc<RotationBits>(w)) {}

  inline void
  RotationCountClasses::extend_bit(RotationBits count, WordValue bit,
                                   unsigned int weight) {
    for (unsigned int i=0; i<width; i++) next[i]={1,0};
    const bool allows_zero=(count.lo & bit)==0;
    const bool allows_one=(count.hi & bit)!=0;
    for (unsigned int i=0; i<width; i++) {
      if (is_empty(current[i])) continue;
      if (allows_zero) next[i]=merge_bits(next[i],current[i]);
      if (allows_one) {
        const unsigned int residue=(i+weight)%width;
        const RotationBits extended={current[i].lo | bit,current[i].hi | bit};
        next[residue]=merge_bits(next[residue],extended);
      }
    }
    std::swap(current,next);
  }

  inline RotationClasses
  RotationCountClasses::compute(RotationBits count) {
    for (unsigned int i=0; i<width; i++) current[i]={1,0};
    current[0]={0,0};
    unsigned int weight=1U%width;
    for (unsigned int i=0; i<width; i++) {
      // Transitions use a bit's residue, never a merged hull as a value.
      extend_bit(count,WordValue(1)<<i,weight);
      weight=(2U*weight)%width;
    }
    WordValue reachable=0;
    for (unsigned int i=0; i<width; i++)
      if (!is_empty(current[i])) reachable |= WordValue(1)<<i;
    return {current,reachable};
  }

  forceinline RotationDomains
  read_domains(RotationViews views) {
    return {{views.input.lo(),views.input.hi()},
            {views.count.lo(),views.count.hi()},
            {views.result.lo(),views.result.hi()}};
  }

  /// Direction and effective amount of one fixed-width bit permutation.
  struct RotationPermutation {
    unsigned int width, amount;
    FixedOp op;
  };

  /// Input and result cubes linked by one rotation.
  struct RotationPair {
    RotationBits input, result;
  };

  forceinline RotationBits
  rotate_bits(RotationBits bits, RotationPermutation permutation) {
    const unsigned int width=permutation.width,amount=permutation.amount;
    const FixedOp op=permutation.op;
    if (op==FO_ROTATE_LEFT)
      return {rotate_left_value(bits.lo,width,amount),
              rotate_left_value(bits.hi,width,amount)};
    return {rotate_right_value(bits.lo,width,amount),
            rotate_right_value(bits.hi,width,amount)};
  }

  /// Cube projection and the separate count reachability/support proofs.
  struct RotationProjectionResult {
    RotationDomains domains;
    WordValue reachable, supported;
  };

  forceinline RotationBits
  project_input(RotationPair pair, RotationPermutation permutation) {
    permutation.op=permutation.op==FO_ROTATE_LEFT ? FO_ROTATE_RIGHT : FO_ROTATE_LEFT;
    const RotationBits backward=rotate_bits(pair.result,permutation);
    return {pair.input.lo | backward.lo,pair.input.hi & backward.hi};
  }

  inline RotationProjectionResult
  project_rotations(RotationDomains domains, RotationClasses classes,
                    unsigned int width, FixedOp op) {
    RotationProjectionResult projection={{{1,0},{1,0},{1,0}},
                                         classes.reachable,0};
    for (unsigned int i=0; i<width; i++) {
      if (is_empty(classes.hulls[i])) continue;
      const RotationPermutation permutation={width,i,op};
      const RotationBits input=project_input({domains.input,domains.result},permutation);
      if (is_empty(input)) continue;
      const RotationBits result=rotate_bits(input,permutation);
      projection.domains.input=merge_bits(projection.domains.input,input);
      projection.domains.count=merge_bits(projection.domains.count,classes.hulls[i]);
      projection.domains.result=merge_bits(projection.domains.result,result);
      projection.supported |= WordValue(1)<<i;
    }
    return projection;
  }

  forceinline ModEvent
  publish_bits(Home home, WordView view, RotationBits bits) {
    const bool is_unchanged=view.lo()==bits.lo && view.hi()==bits.hi;
    return is_unchanged ? ME_WORD_NONE : view.narrow(home,bits.lo,bits.hi);
  }

  forceinline ExecStatus
  publish_domains(Home home, RotationViews views, RotationDomains domains) {
    GECODE_ME_CHECK(publish_bits(home,views.input,domains.input));
    GECODE_ME_CHECK(publish_bits(home,views.count,domains.count));
    GECODE_ME_CHECK(publish_bits(home,views.result,domains.result));
    return ES_OK;
  }

  forceinline bool
  has_same_bits(RotationBits left, RotationBits right) {
    return left.lo==right.lo && left.hi==right.hi;
  }

  forceinline bool
  has_same_domains(RotationDomains left, RotationDomains right) {
    return has_same_bits(left.input,right.input) &&
      has_same_bits(left.count,right.count) && has_same_bits(left.result,right.result);
  }

  /// Stable count proofs or failure from local mask propagation.
  struct RotationClosureResult {
    ExecStatus status;
    WordValue reachable, supported;
  };

  /// Publish stable rotation projections and return their count proofs or failure.
  inline RotationClosureResult
  close_rotation(Home home, RotationViews views, FixedOp op) {
    Region region;
    const unsigned int width=views.input.width();
    RotationCountClasses counts(region,width);
    // Bound synchronization can expose additional bits. Every changing pass
    // fixes at least one of the at most 3*width unknown role bits.
    for (;;) {
      const RotationDomains before=read_domains(views);
      const RotationClasses classes=counts.compute(before.count);
      const RotationProjectionResult projection=project_rotations(before,classes,width,op);
      if (projection.supported==0) return {ES_FAILED,0,0};
      if (publish_domains(home,views,projection.domains)==ES_FAILED)
        return {ES_FAILED,0,0};
      if (has_same_domains(before,read_domains(views)))
        return {ES_OK,projection.reachable,projection.supported};
    }
  }

  forceinline bool
  has_single_residue(WordValue residues) {
    return residues!=0 && (residues & (residues-1))==0;
  }

  forceinline unsigned int
  find_residue(WordValue residues) {
    assert(has_single_residue(residues));
    unsigned int amount=0;
    while ((residues & 1)==0) { residues >>= 1; amount++; }
    return amount;
  }

  forceinline unsigned int
  reduce_count(WordView count) {
    return static_cast<unsigned int>(count.val()%count.width());
  }

  forceinline bool
  is_entailed(WordView input, WordView result, RotationClosureResult closure) {
    return input.assigned() && result.assigned() &&
      closure.reachable==closure.supported;
  }

}}}

// STATISTICS: word-prop
