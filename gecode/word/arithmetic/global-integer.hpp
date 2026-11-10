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

// Private exact arithmetic with 64-bit limbs for arithmetic globals.
#ifndef GECODE_WORD_ARITHMETIC_GLOBAL_INTEGER_HPP
#define GECODE_WORD_ARITHMETIC_GLOBAL_INTEGER_HPP

#include <gecode/support.hh>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {

  // Sign/magnitude base 2^64. Native double-width multiplication when available;
  // the fallback also uses 64-bit limbs. No external dependency or truncation.
  class Integer {
    /*
     * Little-endian magnitude limbs. Up to inline_limbs limbs live in the
     * object; longer sequences spill to a block on Gecode::heap that the
     * object owns. spilled is null exactly when the limbs are stored inline,
     * and capacity is then inline_limbs. Integer::trim removes trailing zero
     * limbs, so a normalised value never ends in a zero limb. Ten limbs hold
     * the exact FP32 products and sums of the floating-point word
     * arithmetic (at most 557 bits) and the long-division buffers for them
     * without heap traffic.
     */
    class Limbs {
      static const size_t inline_limbs=10;
      std::uint64_t local[inline_limbs];
      std::uint64_t* spilled;
      size_t n, capacity;
      std::uint64_t* limbs(void) { return spilled ? spilled : local; }
      const std::uint64_t* limbs(void) const {
        return spilled ? spilled : local;
      }
      // Grows the storage to hold at least m limbs, keeping the live ones.
      // Leaves the object unchanged if the allocation throws.
      void reserve(size_t m) {
        if (m<=capacity) return;
        size_t grown=std::max(m,2*capacity);
        std::uint64_t* block=heap.alloc<std::uint64_t>(grown);
        for (size_t i=0; i<n; i++) block[i]=limbs()[i];
        release();
        spilled=block; capacity=grown;
      }
      void release(void) {
        if (spilled) heap.free<std::uint64_t>(spilled,capacity);
      }
      // Takes over the limbs of o and leaves o empty and inline.
      void steal(Limbs& o) noexcept {
        spilled=o.spilled; n=o.n; capacity=o.capacity;
        if (!spilled)
          for (size_t i=0; i<n; i++) local[i]=o.local[i];
        o.spilled=nullptr; o.n=0; o.capacity=inline_limbs;
      }
    public:
      Limbs(void) : spilled(nullptr), n(0), capacity(inline_limbs) {}
      Limbs(const Limbs& o) : Limbs() {
        reserve(o.n);
        for (size_t i=0; i<o.n; i++) limbs()[i]=o.limbs()[i];
        n=o.n;
      }
      Limbs(Limbs&& o) noexcept { steal(o); }
      Limbs& operator=(const Limbs& o) {
        if (o.n>capacity) return *this=Limbs(o);
        // Self-assignment copies each limb onto itself.
        for (size_t i=0; i<o.n; i++) limbs()[i]=o.limbs()[i];
        n=o.n;
        return *this;
      }
      Limbs& operator=(Limbs&& o) noexcept {
        if (this!=&o) { release(); steal(o); }
        return *this;
      }
      ~Limbs(void) { release(); }
      size_t size(void) const { return n; }
      bool empty(void) const { return n==0; }
      std::uint64_t& operator[](size_t i) { assert(i<n); return limbs()[i]; }
      std::uint64_t operator[](size_t i) const {
        assert(i<n); return limbs()[i];
      }
      std::uint64_t back(void) const { assert(n>0); return limbs()[n-1]; }
      void push_back(std::uint64_t limb) { reserve(n+1); limbs()[n++]=limb; }
      void pop_back(void) { assert(n>0); n--; }
      // Replaces the contents by m zero limbs.
      void zeros(size_t m) {
        n=0; reserve(m);
        for (size_t i=0; i<m; i++) limbs()[i]=0;
        n=m;
      }
    };
    Limbs a;
    bool negative;
    void trim(void) {
      while (!a.empty() && a.back()==0) a.pop_back();
      if (a.empty()) negative=false;
    }
    static int compare_abs(const Integer& x, const Integer& y) {
      if (x.a.size()!=y.a.size()) return x.a.size()<y.a.size() ? -1 : 1;
      for (size_t i=x.a.size(); i--;)
        if (x.a[i]!=y.a[i]) return x.a[i]<y.a[i] ? -1 : 1;
      return 0;
    }
    static std::uint64_t multiply(std::uint64_t x,std::uint64_t y,
                                  std::uint64_t& high) {
#if defined(__SIZEOF_INT128__)
      unsigned __int128 p=static_cast<unsigned __int128>(x)*y;
      high=static_cast<std::uint64_t>(p>>64);
      return static_cast<std::uint64_t>(p);
#elif defined(_MSC_VER) && defined(_M_X64)
      return _umul128(x,y,&high);
#else
      std::uint64_t low=0; high=0;
      for (unsigned int k=0;k<64;k++) if ((y>>k)&1) {
        std::uint64_t term=x<<k, previous=low;
        low+=term; high+=(k ? x>>(64-k) : 0)+(low<previous);
      }
      return low;
#endif
    }
    /// Number of leading zero bits of the nonzero limb \a x
    static unsigned int leading_zeros(std::uint64_t x) {
      assert(x!=0);
#if defined(__GNUC__) || defined(__clang__)
      return static_cast<unsigned int>(__builtin_clzll(x));
#else
      unsigned int n=0;
      for (; !(x>>63); x<<=1) n++;
      return n;
#endif
    }
    /**
     * Return the quotient of \a high * 2^64 + \a low by \a d and set
     * \a rest to the remainder. Requires \a high < \a d and the top bit
     * of \a d set, so the quotient fits one limb.
     */
    static std::uint64_t divide_limb(std::uint64_t high, std::uint64_t low,
                                     std::uint64_t d, std::uint64_t& rest) {
      assert(high<d && (d>>63)==1);
#if defined(__SIZEOF_INT128__)
      const unsigned __int128 v=(static_cast<unsigned __int128>(high)<<64)|low;
      rest=static_cast<std::uint64_t>(v%d);
      return static_cast<std::uint64_t>(v/d);
#else
      // Two steps of long division on 32-bit half limbs (Hacker's Delight,
      // divlu). Each estimate is at most two too large and is corrected
      // against the low half of d; the products are evaluated only for
      // estimates below 2^32, so they cannot overflow.
      const std::uint64_t base=std::uint64_t(1)<<32, mask=base-1;
      const std::uint64_t d1=d>>32, d0=d&mask;
      const std::uint64_t l1=low>>32, l0=low&mask;
      std::uint64_t q1=high/d1, rhat=high-q1*d1;
      while (q1>=base || q1*d0>((rhat<<32)|l1)) {
        q1--; rhat+=d1;
        if (rhat>=base) break;
      }
      // The partial remainder is below d, so the wrapped arithmetic is exact.
      const std::uint64_t middle=(high<<32)+l1-q1*d;
      std::uint64_t q0=middle/d1;
      rhat=middle-q0*d1;
      while (q0>=base || q0*d0>((rhat<<32)|l0)) {
        q0--; rhat+=d1;
        if (rhat>=base) break;
      }
      rest=(middle<<32)+l0-q0*d;
      return (q1<<32)|q0;
#endif
    }
    /**
     * Set \a q and \a r to the quotient and remainder of the magnitudes
     * \a u and \a v, which are trimmed with \a u >= \a v > 0. The results
     * may have leading zero limbs.
     *
     * This is long division on 64-bit limbs (Knuth, TAOCP vol. 2, 4.3.1,
     * algorithm D). Both operands are shifted so that the top bit of the
     * divisor is set. Each quotient limb is then estimated from the top two
     * dividend limbs and the top divisor limb, corrected with the second
     * divisor limb (leaving it at most one too large), and fixed by adding
     * the divisor back when the multiply-and-subtract step goes negative.
     */
    static void divide_magnitude(const Limbs& u, const Limbs& v,
                                 Limbs& q, Limbs& r) {
      assert(!v.empty() && u.size()>=v.size());
      const size_t n=v.size(), m=u.size()-n;
      const unsigned int s=leading_zeros(v.back());
      Limbs vn; vn.zeros(n);
      for (size_t i=0; i<n; i++)
        vn[i]=(v[i]<<s) | (s && i ? v[i-1]>>(64-s) : 0);
      // The shifted dividend, with one more limb, becomes the remainder.
      r.zeros(u.size()+1);
      r[u.size()]=s ? u.back()>>(64-s) : 0;
      for (size_t i=0; i<u.size(); i++)
        r[i]=(u[i]<<s) | (s && i ? u[i-1]>>(64-s) : 0);
      q.zeros(m+1);
      if (n==1) {
        std::uint64_t rest=r[u.size()];
        for (size_t i=u.size(); i--;)
          q[i]=divide_limb(rest,r[i],vn[0],rest);
        r.zeros(1); r[0]=rest>>s;
        return;
      }
      const std::uint64_t top=vn[n-1], second=vn[n-2];
      for (size_t j=m+1; j--;) {
        // Estimate qhat from the top two limbs; it is at most two too large.
        std::uint64_t qhat, rhat;
        bool rhat_overflow=false;
        if (r[j+n]>=top) {
          // r[j+n]==top: the estimate would be 2^64, so use 2^64-1.
          qhat=~std::uint64_t(0);
          rhat=r[j+n-1]+top; rhat_overflow=rhat<top;
        } else {
          qhat=divide_limb(r[j+n],r[j+n-1],top,rhat);
        }
        // Correct with the second divisor limb while rhat fits one limb.
        while (!rhat_overflow) {
          std::uint64_t high, low=multiply(qhat,second,high);
          if (high<rhat || (high==rhat && low<=r[j+n-2]))
            break;
          qhat--; rhat+=top; rhat_overflow=rhat<top;
        }
        // Subtract qhat times the divisor from r[j..j+n].
        std::uint64_t carry=0, borrow=0;
        for (size_t i=0; i<n; i++) {
          std::uint64_t high, low=multiply(qhat,vn[i],high);
          low+=carry; high+=(low<carry); carry=high;
          const std::uint64_t difference=r[i+j]-low;
          const std::uint64_t below=r[i+j]<low;
          r[i+j]=difference-borrow;
          borrow=below | (difference<borrow);
        }
        const std::uint64_t difference=r[j+n]-carry;
        const bool below=r[j+n]<carry || difference<borrow;
        r[j+n]=difference-borrow;
        if (below) {
          // qhat was one too large: add the divisor back once.
          qhat--;
          std::uint64_t add_carry=0;
          for (size_t i=0; i<n; i++) {
            const std::uint64_t sum=r[i+j]+vn[i], total=sum+add_carry;
            add_carry=(sum<vn[i]) || (total<sum);
            r[i+j]=total;
          }
          r[j+n]+=add_carry;
        }
        q[j]=qhat;
      }
      // Undo the normalisation of the remainder held in r[0..n-1].
      for (size_t i=0; i<n; i++)
        r[i]=(r[i]>>s) | (s && i+1<n ? r[i+1]<<(64-s) : 0);
      while (r.size()>n) r.pop_back();
    }
  public:
    Integer(void) : negative(false) {}
    explicit Integer(std::uint64_t x) : negative(false) {
      if (x) a.push_back(x);
    }
    /// Return 2^\a k
    static Integer power_two(unsigned int k) {
      Integer r; r.a.zeros(k/64+1); r.a[k/64]=std::uint64_t(1)<<(k%64);
      return r;
    }
    static Integer signed_value(std::int64_t x) {
      Integer r(x<0 ? std::uint64_t(0)-std::uint64_t(x) : std::uint64_t(x));
      r.negative=x<0; return r;
    }
    bool zero(void) const { return a.empty(); }
    bool neg(void) const { return negative; }
    Integer abs(void) const { Integer r=*this; r.negative=false; return r; }
    Integer operator-(void) const {
      Integer r=*this; if (!r.zero()) r.negative=!r.negative; return r;
    }
    unsigned int bits(void) const {
      if (zero()) return 0;
      return static_cast<unsigned int>(a.size()*64)-leading_zeros(a.back());
    }
    bool bit(unsigned int i) const {
      return i/64<a.size() && ((a[i/64]>>(i%64))&1);
    }
    /// Return this value times 2^\a k
    Integer shift_left(unsigned int k) const {
      Integer r;
      if (zero()) return r;
      const size_t limbs=k/64;
      const unsigned int s=k%64;
      r.a.zeros(a.size()+limbs+1);
      for (size_t i=0; i<a.size(); i++) {
        r.a[i+limbs]|=a[i]<<s;
        if (s) r.a[i+limbs+1]=a[i]>>(64-s);
      }
      r.negative=negative; r.trim(); return r;
    }
    /**
     * Return this value divided by 2^\a k, truncated toward zero: the
     * magnitude is shifted right and the sign is kept. Equals
     * *this / power_two(k).
     */
    Integer shift_right(unsigned int k) const {
      Integer r;
      const size_t limbs=k/64;
      if (limbs>=a.size()) return r;
      const unsigned int s=k%64;
      const size_t n=a.size()-limbs;
      r.a.zeros(n);
      for (size_t i=0; i<n; i++)
        r.a[i]=(a[i+limbs]>>s) | (s && i+1<n ? a[i+limbs+1]<<(64-s) : 0);
      r.negative=negative; r.trim(); return r;
    }
    /// Whether the magnitude is a multiple of 2^\a k
    bool low_bits_zero(unsigned int k) const {
      const size_t limbs=k/64;
      for (size_t i=0; i<limbs && i<a.size(); i++)
        if (a[i]) return false;
      return limbs>=a.size() ||
        (a[limbs] & ((std::uint64_t(1)<<(k%64))-1))==0;
    }
    /**
     * Return the low \a k bits of the magnitude with the sign of this
     * value. Equals *this % power_two(k).
     */
    Integer low_bits(unsigned int k) const {
      Integer r;
      const size_t limbs=std::min<size_t>(k/64+1,a.size());
      r.a.zeros(limbs);
      for (size_t i=0; i<limbs; i++) r.a[i]=a[i];
      if (k/64<a.size()) r.a[k/64]&=(std::uint64_t(1)<<(k%64))-1;
      r.negative=negative; r.trim(); return r;
    }
    std::uint64_t word(void) const {
      assert(!negative && a.size()<=1);
      return zero() ? 0 : a[0];
    }
    friend int compare(const Integer& x, const Integer& y) {
      if (x.negative!=y.negative) return x.negative ? -1 : 1;
      return (x.negative ? -1 : 1)*compare_abs(x,y);
    }
    friend bool operator==(const Integer& x,const Integer& y) {return compare(x,y)==0;}
    friend bool operator!=(const Integer& x,const Integer& y) {return compare(x,y)!=0;}
    friend bool operator<(const Integer& x,const Integer& y) {return compare(x,y)<0;}
    friend bool operator>(const Integer& x,const Integer& y) {return compare(x,y)>0;}
    friend bool operator<=(const Integer& x,const Integer& y) {return compare(x,y)<=0;}
    friend bool operator>=(const Integer& x,const Integer& y) {return compare(x,y)>=0;}
    friend Integer operator+(const Integer& x, const Integer& y) {
      Integer r;
      if (x.negative==y.negative) {
        r.negative=x.negative;
        const size_t n=std::max(x.a.size(),y.a.size());
        r.a.zeros(n+1);
        std::uint64_t carry=0;
        for (size_t i=0; i<n; i++) {
          std::uint64_t u=i<x.a.size() ? x.a[i] : 0;
          std::uint64_t v=i<y.a.size() ? y.a[i] : 0;
          std::uint64_t sum=u+v, result=sum+carry;
          carry=(sum<u) || (result<sum); r.a[i]=result;
        }
        r.a[n]=carry;
      } else {
        const bool swap=compare_abs(x,y)<0;
        const Integer& u=swap ? y : x;
        const Integer& v=swap ? x : y;
        r.negative=u.negative;
        r.a.zeros(u.a.size());
        std::uint64_t borrow=0;
        for (size_t i=0; i<u.a.size(); i++) {
          std::uint64_t term=i<v.a.size() ? v.a[i] : 0, sub=term+borrow;
          r.a[i]=u.a[i]-sub;
          borrow=(sub<term) || (u.a[i]<sub);
        }
      }
      r.trim(); return r;
    }
    friend Integer operator-(const Integer& x,const Integer& y) { return x+(-y); }
    friend Integer operator*(const Integer& x, const Integer& y) {
      Integer r; r.a.zeros(x.a.size()+y.a.size());
      r.negative=x.negative!=y.negative;
      for (size_t i=0; i<x.a.size(); i++) {
        std::uint64_t carry=0;
        for (size_t j=0; j<y.a.size(); j++) {
          std::uint64_t high,low=multiply(x.a[i],y.a[j],high);
          std::uint64_t sum=low+r.a[i+j]; high+=(sum<low);
          low=sum+carry; high+=(low<sum);
          r.a[i+j]=low; carry=high;
        }
        if (!y.a.empty()) r.a[i+y.a.size()]=carry;
      }
      r.trim(); return r;
    }
    /**
     * Set \a q and \a r to the quotient and remainder of \a x by the
     * nonzero \a y, truncating toward zero: \a q is negative exactly when
     * it is nonzero and the signs differ, and a nonzero \a r has the sign
     * of \a x. \a q and \a r must be distinct objects; they may alias
     * \a x or \a y.
     */
    static void divide(const Integer& x, const Integer& y, Integer& q,Integer& r) {
      assert(!y.zero());
      Integer quotient, rest;
      if (compare_abs(x,y)<0) {
        rest=x;
      } else {
        divide_magnitude(x.a,y.a,quotient.a,rest.a);
        quotient.trim(); rest.trim();
      }
      quotient.negative=!quotient.zero() && (x.negative!=y.negative);
      rest.negative=!rest.zero() && x.negative;
      q=std::move(quotient); r=std::move(rest);
    }
    friend Integer operator/(const Integer& x,const Integer& y) {
      Integer q,r; divide(x,y,q,r); return q;
    }
    friend Integer operator%(const Integer& x,const Integer& y) {
      Integer q,r; divide(x,y,q,r); return r;
    }
  };
  inline Integer floor_div(const Integer& x,const Integer& y) {
    Integer q,r; Integer::divide(x,y,q,r);
    return !r.zero() && x.neg()!=y.neg() ? q-Integer(1) : q;
  }
  inline Integer ceil_div(const Integer& x,const Integer& y) {
    return -floor_div(-x,y);
  }
  /**
   * Return the floor of \a x / 2^\a k, which equals
   * floor_div(x,Integer::power_two(k)). Integer::shift_right truncates
   * toward zero instead, so a negative \a x that is no multiple of 2^\a k
   * takes one more step down.
   */
  inline Integer floor_shift_right(const Integer& x,unsigned int k) {
    const Integer q=x.shift_right(k);
    return x.neg() && !x.low_bits_zero(k) ? q-Integer(1) : q;
  }
  /**
   * Return the ceiling of \a x / 2^\a k, which equals
   * ceil_div(x,Integer::power_two(k)): a positive \a x that is no multiple
   * of 2^\a k takes one step up from the truncated quotient.
   */
  inline Integer ceil_shift_right(const Integer& x,unsigned int k) {
    const Integer q=x.shift_right(k);
    return !x.neg() && !x.low_bits_zero(k) ? q+Integer(1) : q;
  }
  inline Integer mod(const Integer& x,const Integer& m) {
    Integer r=x%m; return r.neg() ? r+m : r;
  }
  inline Integer gcd(Integer a,Integer b) {
    a=a.abs(); b=b.abs();
    while (!b.zero()) { Integer r=a%b; a=b; b=r; }
    return a;
  }
  // Requires gcd(a,m)=1, m>1.
  inline Integer inverse(Integer a,const Integer& m) {
    Integer b=m, u(1), v;
    a=mod(a,m);
    while (!b.zero()) {
      Integer q=a/b, r=a-q*b, t=u-q*v;
      a=b; b=r; u=v; v=t;
    }
    assert(a==Integer(1)); return mod(u,m);
  }

}}}}
#endif

// STATISTICS: word-prop
