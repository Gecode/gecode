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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_PROGRESSION_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_PROGRESSION_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline WordValue
  compute_bound_gcd(WordValue x, WordValue y) {
    while (y != 0U) {
      const WordValue r=x%y; x=y; y=r;
    }
    return x;
  }

  /// Cube bits in rank order, with the word's width and mask.
  struct BoundOrderedCube {
    unsigned int width;
    WordValue lo, hi, mask;
  };

  /** \brief Ranked arithmetic progression within one word
   * A zero step denotes one rank; is_empty denotes no compatible rank.
   */
  struct BoundRankProgression {
    WordValue first, step;
    bool is_empty;
  };

  /// Fixed low bits of a cube, represented by their width and value.
  struct BoundBitPrefix {
    unsigned int width;
    WordValue value;
  };

  forceinline BoundBitPrefix
  find_bound_prefix(const BoundOrderedCube& cube) {
    const WordValue unknown=(cube.lo^cube.hi)&cube.mask;
    unsigned int prefix_width=0;
    for (;;) {
      const bool has_fixed_prefix_bit=(prefix_width < cube.width) &&
        ((unknown&(WordValue(1)<<prefix_width)) == 0U);
      if (!has_fixed_prefix_bit)
        break;
      prefix_width++;
    }
    return {prefix_width,cube.lo&width_mask(prefix_width)};
  }

  forceinline WordValue
  compute_bound_inverse(WordValue coefficient, unsigned int prefix_width) {
    // Removing gcd(step,2^prefix_width) leaves an odd coefficient.
    // Newton refinement doubles the inverse's correct low bits each round.
    WordValue inverse=coefficient;
    for (unsigned int bits=1; bits<prefix_width; bits <<= 1)
      inverse *= 2U-coefficient*inverse;
    return inverse;
  }

  forceinline BoundRankProgression
  solve_bound_congruence(BoundBitPrefix prefix, unsigned int width,
                          WordValue mask, WordValue residue, WordValue step) {
    if (prefix.width == width) {
      if ((prefix.value%step) != residue)
        return {0U,0U,true};
      return {prefix.value,0U,false};
    }
    if (prefix.width == 0U)
      return {residue,step,false};
    const WordValue prefix_modulus=WordValue(1)<<prefix.width;
    const WordValue prefix_mask=prefix_modulus-1U;
    const WordValue divisor=compute_bound_gcd(step,prefix_modulus);
    const WordValue difference=(prefix.value-(residue&prefix_mask))&prefix_mask;
    const bool is_prefix_compatible=(difference&(divisor-1U)) == 0U;
    if (!is_prefix_compatible)
      return {0U,0U,true};
    const WordValue reduced_modulus=prefix_modulus/divisor;
    WordValue multiplier=0U;
    if (reduced_modulus != 1U) {
      const WordValue reduced_mask=reduced_modulus-1U;
      const WordValue coefficient=(step/divisor)&reduced_mask;
      const WordValue inverse=compute_bound_inverse(coefficient,prefix.width);
      multiplier=((difference/divisor)*inverse)&reduced_mask;
    }
    const bool has_offset_overflow=(multiplier != 0U) && (step > mask/multiplier);
    if (has_offset_overflow)
      return {0U,0U,true};
    const WordValue combined=residue+step*multiplier;
    if (combined < residue)
      return {0U,0U,true};
    const bool has_representable_step=step <= mask/reduced_modulus;
    const WordValue combined_step=has_representable_step ?
      step*reduced_modulus : 0U;
    return {combined,combined_step,false};
  }

  /// Combine the requested congruence with the cube's fixed low-bit prefix.
  forceinline BoundRankProgression
  combine_bound_progression(const BoundOrderedCube& cube,
                            WordValue residue, WordValue step) {
    assert(step != 0U);
    residue %= step;
    const BoundBitPrefix prefix=find_bound_prefix(cube);
    return solve_bound_congruence(prefix,cube.width,cube.mask,residue,step);
  }

  forceinline WordIntervalResult
  align_bound_progression(WordRankInterval interval,
                          const BoundRankProgression& progression,
                          WordValue mask) {
    WordValue first=progression.first;
    if (first < interval.minimum) {
      if (progression.step == 0U)
        return {false,{0,0}};
      const WordValue delta=interval.minimum-first;
      const WordValue count=delta/progression.step +
        ((delta%progression.step) != 0U);
      const bool has_offset_overflow=
        (count != 0U) && (progression.step > (mask-first)/count);
      if (has_offset_overflow)
        return {false,{0,0}};
      first += count*progression.step;
    }
    const bool is_outside_interval=(first < interval.minimum) ||
      (first > interval.maximum);
    if (is_outside_interval)
      return {false,{0,0}};
    WordValue last=first;
    if (progression.step != 0U)
      last += ((interval.maximum-first)/progression.step)*progression.step;
    return {true,{first,last}};
  }

  /** Intersect supported progression endpoints without searching cube holes.
   * Unsupported endpoints retain the old bounds so synchronization cannot
   * walk an exponentially long sequence of progression/cube mismatches.
   */
  forceinline bool
  intersect_bound_endpoints(BoundLocalDomain& domain,
                             const BoundOrderedCube& cube,
                             const BoundRankProgression& progression) {
    const WordValue mask=cube.mask;
    const WordIntervalResult aligned=align_bound_progression(
      {domain.minimum,domain.maximum},progression,mask);
    if (!aligned.is_admitted)
      return false;
    WordValue first=aligned.interval.minimum, last=aligned.interval.maximum;
    const bool is_first_supported=cube_contains(cube.lo,cube.hi,first,mask);
    const bool is_last_supported=cube_contains(cube.lo,cube.hi,last,mask);
    const bool has_unsupported_singleton=(first == last) && !is_first_supported;
    if (has_unsupported_singleton)
      return false;
    if (!is_first_supported) first=domain.minimum;
    if (!is_last_supported) last=domain.maximum;
    return domain.intersect_range(first,last);
  }

  /// Intersect a local domain with rank == residue (mod step).
  forceinline bool
  bound_progression(BoundLocalDomain& domain, WordValue residue,
                              WordValue step) {
    const WordCube ordered=order_cube(domain.kind,domain.width,
                                      {domain.lo,domain.hi});
    const BoundOrderedCube cube={
      domain.width,ordered.lo,ordered.hi,width_mask(domain.width)};
    const BoundRankProgression progression=
      combine_bound_progression(cube,residue,step);
    return !progression.is_empty &&
      intersect_bound_endpoints(domain,cube,progression);
  }

}}}

#endif
