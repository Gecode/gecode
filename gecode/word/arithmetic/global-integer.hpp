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
     * limbs, so a normalised value never ends in a zero limb.
     */
    class Limbs {
      static const size_t inline_limbs=4;
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
    void shift_bit(bool bit) {
      std::uint64_t carry=bit;
      for (size_t i=0; i<a.size(); i++) {
        std::uint64_t next=a[i]>>63;
        a[i]=(a[i]<<1)|carry; carry=next;
      }
      if (carry) a.push_back(carry);
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
  public:
    Integer(void) : negative(false) {}
    explicit Integer(std::uint64_t x) : negative(false) {
      if (x) a.push_back(x);
    }
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
      unsigned int n=static_cast<unsigned int>((a.size()-1)*64);
      for (std::uint64_t v=a.back(); v; v>>=1) n++;
      return n;
    }
    bool bit(unsigned int i) const {
      return i/64<a.size() && ((a[i/64]>>(i%64))&1);
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
        std::uint64_t carry=0;
        for (size_t i=0; i<std::max(x.a.size(),y.a.size()); i++) {
          std::uint64_t u=i<x.a.size() ? x.a[i] : 0;
          std::uint64_t v=i<y.a.size() ? y.a[i] : 0;
          std::uint64_t sum=u+v, result=sum+carry;
          carry=(sum<u) || (result<sum); r.a.push_back(result);
        }
        if (carry) r.a.push_back(carry);
      } else {
        const bool swap=compare_abs(x,y)<0;
        const Integer& u=swap ? y : x;
        const Integer& v=swap ? x : y;
        r.negative=u.negative;
        std::uint64_t borrow=0;
        for (size_t i=0; i<u.a.size(); i++) {
          std::uint64_t term=i<v.a.size() ? v.a[i] : 0, sub=term+borrow;
          r.a.push_back(u.a[i]-sub);
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
    static void divide(const Integer& x, const Integer& y, Integer& q,Integer& r) {
      assert(!y.zero());
      q=Integer(); r=Integer(); const Integer d=y.abs();
#if defined(__SIZEOF_INT128__)
      if (d.a.size()==1) {
        q.a.zeros(x.a.size()); std::uint64_t remainder=0;
        for (size_t i=x.a.size();i--;) {
          unsigned __int128 v=(static_cast<unsigned __int128>(remainder)<<64)|x.a[i];
          q.a[i]=static_cast<std::uint64_t>(v/d.a[0]);
          remainder=static_cast<std::uint64_t>(v%d.a[0]);
        }
        q.trim(); r=Integer(remainder);
        q.negative=!q.zero() && (x.negative!=y.negative);
        r.negative=!r.zero() && x.negative;
        return;
      }
#endif
      for (unsigned int i=x.bits(); i--;) {
        r.shift_bit(x.bit(i));
        bool take=r>=d;
        if (take) r=r-d;
        q.shift_bit(take);
      }
      q.negative=!q.zero() && (x.negative!=y.negative);
      r.negative=!r.zero() && x.negative;
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
