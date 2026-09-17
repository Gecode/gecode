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

#ifndef GECODE_WORD_ARITHMETIC_GLOBAL_DOMAIN_HPP
#define GECODE_WORD_ARITHMETIC_GLOBAL_DOMAIN_HPP

#include <gecode/word/arithmetic/global-integer.hpp>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {

  const Integer zero, one(1);
  // Convert an interval between unsigned and signed rank orders. Flipping
  // the sign bit splits it into at most two intervals. Cube endpoint queries
  // compute their hull without visiting any represented values.
  bool rank_hull(unsigned int width,WordValue lo,WordValue hi,
                 WordValue lower,WordValue upper,WordValue source_sign,
                 WordValue target_sign,WordValue& first,WordValue& last) {
    WordValue mask=width_mask(width),ol,oh;
    ordered_cube(target_sign ? WDT_SIGNED : WDT_UNSIGNED,width,lo,hi,ol,oh);
    bool found=false;
    auto part=[&](WordValue l,WordValue u) {
      WordValue a,b;
      if (l>u || !cube_successor(ol,oh,l,mask,a) || a>u ||
          !cube_predecessor(ol,oh,u,mask,b) || b<a) return;
      first=found ? std::min(first,a) : a;
      last=found ? std::max(last,b) : b;
      found=true;
    };
    if (source_sign==target_sign) part(lower,upper);
    else {
      const WordValue bit=WordValue(1)<<(width-1);
      if (lower<bit) part(lower^bit,std::min(upper,bit-1)^bit);
      if (upper>=bit) part(std::max(lower,bit)^bit,upper^bit);
    }
    return found;
  }
  struct Domain {
    WordValue lo,hi,mask,sign;
    unsigned int width;
    WordDomainType kind;
    Integer lower,upper,residue,step;
    Domain(WordView x,bool signed_value)
      : lo(x.lo()),hi(x.hi()),mask(x.mask()),
        sign(signed_value ? WordValue(1)<<(x.width()-1) : 0),
        width(x.width()),kind(signed_value ? WDT_SIGNED : WDT_UNSIGNED),step(1) {
      WordValue first=0,last=0;
      bool ok=rank_hull(width,lo,hi,x.bounded() ? x.rank_minimum() : 0,
        x.bounded() ? x.rank_maximum() : mask,
        x.domain_type()==WDT_SIGNED ? WordValue(1)<<(width-1) : 0,sign,first,last);
      assert(ok); (void) ok;
      lower=Integer(first)-Integer(sign); upper=Integer(last)-Integer(sign);
    }
    ModEvent publish(Space& home,WordView x) const {
      if (!x.bounded()) return x.narrow(home,lo,hi);
      WordValue first=0,last=0;
      if (!rank_hull(width,lo,hi,lower_rank(),upper_rank(),sign,
            x.domain_type()==WDT_SIGNED ? WordValue(1)<<(width-1) : 0,first,last))
        return ME_WORD_FAILED;
      return x.domain_type()==WDT_SIGNED ?
        SignedWordView(x.varimp()).narrow_domain(home,lo,hi,first,last) :
        UnsignedWordView(x.varimp()).narrow_domain(home,lo,hi,first,last);
    }
    Integer number(WordValue encoded) const {
      return sign && (encoded&sign) ? Integer(encoded)-Integer::power_two(width) :
        Integer(encoded);
    }
    Integer base(void) const { return number(lo); }
    WordValue lower_rank(void) const { return (lower+Integer(sign)).word(); }
    WordValue upper_rank(void) const { return (upper+Integer(sign)).word(); }
    bool sync(void) {
      if ((lo & ~hi) || lower>upper) return false;
      WordValue first,last,ordered_lo,ordered_hi;
      ordered_cube(kind,width,lo,hi,ordered_lo,ordered_hi);
      if (!cube_successor(ordered_lo,ordered_hi,lower_rank(),mask,first) ||
          first>upper_rank() ||
          !cube_predecessor(ordered_lo,ordered_hi,upper_rank(),mask,last) || last<first) return false;
      lower=Integer(first)-Integer(sign); upper=Integer(last)-Integer(sign);
      WordValue varying=first^last;
      varying|=varying>>1; varying|=varying>>2; varying|=varying>>4;
      varying|=varying>>8; varying|=varying>>16; varying|=varying>>32;
      ordered_lo|=first&~varying; ordered_hi&=last|varying;
      encoded_cube(kind,width,ordered_lo,ordered_hi,lo,hi);
      return true;
    }
    bool range(Integer l,Integer u) {
      l=std::max(l,Integer(lower)); u=std::min(u,Integer(upper));
      if (l>u) return false;
      lower=l; upper=u; return sync();
    }
    bool assigned(void) const { return lower==upper; }
    unsigned int prefix(void) const {
      WordValue unknown=lo^hi;
      unsigned int k=0;
      while (k<64 && !(unknown & (WordValue(1)<<k))) k++;
      return k;
    }
  };
  typedef std::vector<Domain> Domains;

  // Intersect a*x=b (mod m). The unary progression is kept for this pass;
  // only its interval/bit hull can be published to a WordVar.
  bool congruence(Domain& x,Integer a,Integer b,Integer m) {
    assert(m>zero);
    Integer g=gcd(a,m);
    if (!mod(b,g).zero()) return false;
    m=m/g;
    if (m==one) return true;
    Integer r=mod((b/g)*inverse(a/g,m),m);
    // Generalized CRT also joins different rows within this propagation pass.
    g=gcd(x.step,m);
    if (!mod(r-x.residue,g).zero()) return false;
    Integer reduced=m/g;
    Integer multiple=reduced==one ? zero :
      mod(((r-x.residue)/g)*inverse(x.step/g,reduced),reduced);
    r=x.residue+x.step*multiple; m=x.step*reduced; r=mod(r,m);
    x.residue=r; x.step=m;
    Integer first=r+ceil_div(Integer(x.lower)-r,m)*m;
    Integer last=r+floor_div(Integer(x.upper)-r,m)*m;
    if (first>last || !x.range(first,last)) return false;
    // The 2-primary part of any modulus fixes a low bit prefix.
    unsigned int k=0;
    while (k<64 && !m.bit(k)) k++;
    k=std::min(k,x.width);
    WordValue bits=k==64 ? ~WordValue(0) : (WordValue(1)<<k)-1;
    WordValue residue=mod(r,Integer::power_two(k)).word();
    x.lo|=residue; x.hi&=~bits|residue;
    return x.sync();
  }

}}}}

#endif

// STATISTICS: word-prop
