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

#ifndef GECODE_BENCHMARK_WORD_SMT2_CARDINALITY_HPP
#define GECODE_BENCHMARK_WORD_SMT2_CARDINALITY_HPP

#include <gecode/word.hh>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace WordSmt2 {

  /// Exact counts from zero through 2^64, without a compiler-specific integer.
  /// Arithmetic operands and results must remain in this closed interval.
  struct Cardinality {
    std::uint64_t low;
    unsigned int high;
    Cardinality(std::uint64_t n=0) : low(n), high(0) {}
    static Cardinality power(unsigned int bits) {
      Cardinality n;
      if (bits == 64) n.high=1;
      else n.low=std::uint64_t(1) << bits;
      return n;
    }
    Cardinality& operator +=(Cardinality n) {
      const std::uint64_t previous=low;
      low+=n.low;
      high+=n.high+(low < previous);
      assert(high <= 1 && (high == 0 || low == 0));
      return *this;
    }
    Cardinality operator -(Cardinality n) const {
      Cardinality result(low-n.low);
      result.high=high-n.high-(low < n.low);
      assert(result.high <= 1 && (result.high == 0 || result.low == 0));
      return result;
    }
    bool operator ==(Cardinality n) const {
      return low == n.low && high == n.high;
    }
    bool operator !=(Cardinality n) const { return !(*this == n); }
    explicit operator double(void) const {
      return high ? std::ldexp(1.0,64) : static_cast<double>(low);
    }
  };

  inline unsigned int popcount(std::uint64_t value) {
    // Portable parallel population count on an exactly 64-bit unsigned type.
    value-=((value >> 1) & UINT64_C(0x5555555555555555));
    value=(value & UINT64_C(0x3333333333333333)) +
      ((value >> 2) & UINT64_C(0x3333333333333333));
    value=(value+(value >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
    return static_cast<unsigned int>((value*UINT64_C(0x0101010101010101)) >> 56);
  }

  inline Cardinality cube_count(std::uint64_t unknown) {
    return Cardinality::power(popcount(unknown));
  }

  inline Cardinality prefix_count(unsigned int width, std::uint64_t lo,
                                  std::uint64_t hi, std::uint64_t limit) {
    if (limit < lo) return 0;
    const std::uint64_t unknown=hi & ~lo;
    if (limit >= hi) return cube_count(unknown);
    // Bits above the highest unknown bit form a fixed, matching prefix.
    // The guards above imply unknown != 0.
    unsigned int varying_width=1;
    std::uint64_t top=unknown;
    for (unsigned int step=32; step != 0; step >>= 1)
      if ((top >> step) != 0) { top >>= step; varying_width+=step; }
    (void) width;
    Cardinality total;
    for (unsigned int bit=varying_width; bit-- > 0;) {
      const std::uint64_t mask=std::uint64_t(1) << bit;
      if ((limit & mask) != 0) {
        if ((lo & mask) == 0)
          total+=cube_count(unknown & (mask-1));
        if ((hi & mask) == 0) return total;
      } else if ((lo & mask) != 0) return total;
    }
    total+=1;
    return total;
  }

  /// Count cube values inside the requested signed or unsigned interval.
  /// Width is in 1..64; all masks and endpoints use that width's encoding.
  inline Cardinality cardinality(unsigned int width, std::uint64_t lo,
                                  std::uint64_t hi, std::uint64_t minimum,
                                  std::uint64_t maximum, bool signed_order) {
    const std::uint64_t sign=signed_order ? std::uint64_t(1) << (width-1) : 0;
    const std::uint64_t unknown=hi & ~lo;
    const std::uint64_t rank_lo=(lo^sign)&~unknown;
    const std::uint64_t rank_hi=rank_lo|unknown;
    std::uint64_t lower=minimum^sign,upper=maximum^sign;
    if (lower > upper) return 0;
    if (lower < rank_lo) lower=rank_lo;
    if (upper > rank_hi) upper=rank_hi;
    if (lower > upper) return 0;
    if (lower == rank_lo && upper == rank_hi) return cube_count(unknown);
    // Contiguous unknown low bits describe a complete numeric interval.
    // Unsigned wrap in unknown+1 also recognizes the full 64-bit cube.
    if ((unknown & (unknown+std::uint64_t(1))) == 0) {
      Cardinality result(upper-lower);
      result+=1;
      return result;
    }
    const Cardinality below=lower == 0 ? Cardinality() :
      prefix_count(width,rank_lo,rank_hi,lower-1);
    return prefix_count(width,rank_lo,rank_hi,upper)-below;
  }

  /// Select bounded words by AFC divided by their current exact cardinality.
  /// All variables must use WDT_SIGNED or WDT_UNSIGNED domains.
  inline Gecode::WordVarBranch bounded_afc(Gecode::Home home,
                                           const Gecode::WordVarArgs& variables) {
    using namespace Gecode;
    WordVarBranch selector=WORD_VAR_MERIT_MAX(
      [](const Space&, WordVar x, int) -> double {
        const Cardinality count=cardinality(x.width(),x.lo(),x.hi(),
          x.minimum(),x.maximum(),x.domain_type() == WDT_SIGNED);
        assert(count != Cardinality());
        return x.afc()/static_cast<double>(count);
      });
    // Initialize space-managed AFC counters. The merit callback captures no
    // state and reads each current variable, including after cloning.
    selector.afc(WordAFC(home,variables,1.0));
    return selector;
  }

}
#endif
