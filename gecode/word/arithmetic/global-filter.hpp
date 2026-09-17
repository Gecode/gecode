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

#ifndef GECODE_WORD_ARITHMETIC_GLOBAL_FILTER_HPP
#define GECODE_WORD_ARITHMETIC_GLOBAL_FILTER_HPP

#include <gecode/word/arithmetic/global-domain.hpp>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {
  struct LinearRow {
    std::vector<Integer> a;
    Integer lower,upper,modulus;
  };

  // Two-word shift/OR subset-sum filtering over the Boolean bits of an
  // arithmetic row. This stores attainable sums, never variable values or
  // assignment tuples.
  struct SmallSums {
    std::uint64_t low,high;
    SmallSums shifted(unsigned int k) const {
      assert(k<128);
      if (!k) return *this;
      if (k>=64) return {0,low<<(k-64)};
      return {low<<k,(high<<k)|(low>>(64-k))};
    }
    void add(unsigned int weight) {
      SmallSums s=shifted(weight); low|=s.low; high|=s.high;
    }
    bool intersects(const SmallSums& s) const {
      return (low&s.low) || (high&s.high);
    }
    static SmallSums through(unsigned int k) {
      if (k>=127) return {~std::uint64_t(0),~std::uint64_t(0)};
      if (k>=64) return {~std::uint64_t(0),(std::uint64_t(1)<<(k-63))-1};
      return {k==63 ? ~std::uint64_t(0) : (std::uint64_t(1)<<(k+1))-1,0};
    }
  };
  bool small_knapsack(const LinearRow& row,Domains& x) {
    if (!row.modulus.zero()) return true;
    struct Item { size_t variable; unsigned int bit; bool complement; Integer weight; };
    std::vector<Item> items;
    Integer base,g,total;
    for (size_t i=0;i<x.size();i++) {
      if (row.a[i].zero()) continue;
      base=base+row.a[i]*x[i].base();
      for (unsigned int b=0;b<x[i].width;b++) if ((x[i].lo^x[i].hi)&(WordValue(1)<<b)) {
        // Every normalized weight is at least one. More than 127 items cannot
        // fit this representation, irrespective of their common divisor.
        if (items.size()==127) return true;
        Integer delta=row.a[i]*Integer::power_two(b);
        if (x[i].sign && b==x[i].width-1) delta=-delta;
        if (delta.neg()) base=base+delta;
        items.push_back({i,b,delta.neg(),delta.abs()});
        g=gcd(g,delta); total=total+delta.abs();
      }
    }
    if (g.zero()) return base>=row.lower && base<=row.upper;
    total=total/g;
    if (total>Integer(127)) return true;
    Integer lower=std::max(zero,ceil_div(row.lower-base,g));
    Integer upper=std::min(total,floor_div(row.upper-base,g));
    if (lower>upper) return false;
    SmallSums allowed=SmallSums::through(static_cast<unsigned int>(upper.word()));
    if (lower>zero) {
      SmallSums below=SmallSums::through(static_cast<unsigned int>((lower-one).word()));
      allowed.low&=~below.low; allowed.high&=~below.high;
    }
    std::vector<unsigned int> weights;
    SmallSums all={1,0};
    for (const auto& item : items) {
      unsigned int weight=static_cast<unsigned int>((item.weight/g).word());
      weights.push_back(weight); all.add(weight);
    }
    if (!all.intersects(allowed)) return false;
    // Removing an item is not the inverse of shift/OR, so recompute the
    // complement set. At most 127 items can reach here.
    for (size_t i=0;i<items.size();i++) {
      SmallSums rest={1,0};
      for (size_t j=0;j<items.size();j++) if (j!=i) rest.add(weights[j]);
      bool off=rest.intersects(allowed),on=rest.shifted(weights[i]).intersects(allowed);
      if (!off && !on) return false;
      if (!off || !on) {
        bool bit=on!=items[i].complement;
        WordValue mask=WordValue(1)<<items[i].bit;
        if (bit) x[items[i].variable].lo|=mask;
        else x[items[i].variable].hi&=~mask;
      }
    }
    for (auto& d : x) if (!d.sync()) return false;
    return true;
  }
  enum Family { PRODUCT, DIVMOD, LINEAR, RADIX, IMAGE };
  struct Specification {
    Family family;
    bool advanced;
    unsigned int work;
    std::vector<int> left,right;
    std::vector<bool> signed_values;
    Integer c,d;
    int divisor,quotient,remainder;
    bool weighted;
    std::vector<Integer> weights;
    Integer constant;
    std::vector<LinearRow> rows,derived_rows;
    // Fixed-radix digit caps, already mapped to unique views.
    std::vector<std::pair<int,WordValue>> caps;
    struct Field { int digit,rank; unsigned int shift,bits; };
    std::vector<Field> fields;
    Specification(Family f,IntPropLevel ipl)
      : family(f), advanced((ba(ipl)&IPL_ADVANCED)!=0), work(4), c(1),d(1),
      divisor(-1),quotient(-1),remainder(-1),weighted(false) {}
  };
  typedef std::pair<Integer,Integer> Interval;
  Interval multiply(const Interval& a,const Interval& b) {
    Integer p=a.first*b.first,q=a.first*b.second,
      r=a.second*b.first,s=a.second*b.second;
    return {std::min(std::min(p,q),std::min(r,s)),
            std::max(std::max(p,q),std::max(r,s))};
  }
  // Project N=x*A through the negative and positive parts of A. The quotient
  // hull uses outward integer rounding of the four real interval corners.
  bool inverse_product(Domain& x,const Interval& n,const Interval& a) {
    if (a.first<=zero && a.second>=zero && n.first<=zero && n.second>=zero)
      return true; // A=0 supports any x in this interval relaxation.
    bool found=false; Integer lower,upper;
    auto part=[&](Integer l,Integer u) {
      if (l>u) return;
      Integer lo=std::min(std::min(ceil_div(n.first,l),ceil_div(n.first,u)),
                          std::min(ceil_div(n.second,l),ceil_div(n.second,u)));
      Integer hi=std::max(std::max(floor_div(n.first,l),floor_div(n.first,u)),
                          std::max(floor_div(n.second,l),floor_div(n.second,u)));
      if (lo>hi) return;
      lower=found ? std::min(lower,lo) : lo; upper=found ? std::max(upper,hi) : hi;
      found=true;
    };
    part(a.first,std::min(a.second,-one));
    part(std::max(a.first,one),a.second);
    return found && x.range(lower,upper);
  }
  Interval
  product_bounds(const std::vector<int>& factors,const Integer& coefficient,
                 const Domains& x) {
    Interval p={coefficient,coefficient};
    for (size_t j=0;j<factors.size();j++)
      p=multiply(p,{x[factors[j]].lower,x[factors[j]].upper});
    return p;
  }
  bool product_filter(const std::vector<int>& factors,const Integer& coefficient,
                      Domains& x,const Integer& lower,const Integer& upper) {
    auto p=product_bounds(factors,coefficient,x);
    if (p.first>upper || p.second<lower) return false;
    // Prefix/suffix bounds use the entry snapshot. Later pruning can tighten
    // them on the next pass; stale bounds here remain a sound relaxation.
    std::vector<Interval> suffix(factors.size()+1,Interval(one,one));
    for (size_t j=factors.size();j--;)
      suffix[j]=multiply({x[factors[j]].lower,x[factors[j]].upper},suffix[j+1]);
    Interval prefix(coefficient,coefficient);
    for (size_t j=0; j<factors.size(); j++) {
      const Interval current(x[factors[j]].lower,x[factors[j]].upper);
      if (!inverse_product(x[factors[j]],{lower,upper},
                           multiply(prefix,suffix[j+1]))) return false;
      prefix=multiply(prefix,current);
    }
    return true;
  }
  // A conservative common fixed low prefix of an entire product.
  std::pair<Integer,Integer>
  product_prefix(const std::vector<int>& factors,const Integer& coefficient,
                 const Domains& x) {
    unsigned int cv=0;
    while (cv<64 && !coefficient.bit(cv)) cv++;
    std::vector<unsigned int> valuations;
    for (int i : factors) {
      unsigned int v=0;
      while (v<64 && !(x[i].lo&(WordValue(1)<<v))) v++;
      valuations.push_back(std::min(v,x[i].prefix()));
    }
    // Sum valuations once instead of rescanning all other occurrences.
    size_t total=0;
    for (unsigned int v : valuations) total+=v;
    unsigned int k=64; Integer value=coefficient;
    for (size_t i=0;i<factors.size();i++) {
      value=value*x[factors[i]].base();
      if (x[factors[i]].assigned()) continue;
      unsigned int precision=static_cast<unsigned int>(std::min<size_t>(
        64,cv+x[factors[i]].prefix()+total-valuations[i]));
      k=std::min(k,precision);
    }
    Integer m=Integer::power_two(k);
    return {mod(value,m),m};
  }
  bool product_residue(const std::vector<int>& factors,const Integer& coefficient,
                       Domains& x,const std::pair<Integer,Integer>& target) {
    size_t unfixed=0,remaining=0;
    Integer fixed=coefficient;
    for (size_t j=0;j<factors.size();j++) {
      if (x[factors[j]].assigned()) fixed=fixed*x[factors[j]].lower;
      else { unfixed++; remaining=j; }
    }
    if (unfixed==0 && !mod(fixed-target.first,target.second).zero()) return false;
    if (unfixed==1 &&
        !congruence(x[factors[remaining]],fixed,target.first,target.second)) return false;
    auto p=product_prefix(factors,coefficient,x);
    return mod(p.first-target.first,gcd(p.second,target.second)).zero();
  }
  bool product(const Specification& s,Domains& x) {
    auto a=product_bounds(s.left,s.c,x), b=product_bounds(s.right,s.d,x);
    if (!product_filter(s.left,s.c,x,b.first,b.second) ||
        !product_filter(s.right,s.d,x,a.first,a.second)) return false;
    return product_residue(s.left,s.c,x,product_prefix(s.right,s.d,x)) &&
      product_residue(s.right,s.d,x,product_prefix(s.left,s.c,x));
  }

  bool linear_row(const LinearRow& row,Domains& x,bool use_knapsack);

  bool divmod(const Specification& s,Domains& x,bool use_knapsack) {
    if (s.divisor==s.remainder) return false;
    Domain& d=x[s.divisor]; Domain& q=x[s.quotient]; Domain& r=x[s.remainder];
    if (!d.range(one,Integer(d.upper)) ||
        !r.range(zero,Integer(d.upper)-one) ||
        !d.range(Integer(r.lower)+one,Integer(d.upper))) return false;
    auto n=product_bounds(s.left,one,x);
    if (s.weighted) {
      n={s.constant,s.constant};
      for (size_t i=0;i<x.size();i++) {
        Integer l=s.weights[i]*Integer(x[i].lower),u=s.weights[i]*Integer(x[i].upper);
        n.first=n.first+std::min(l,u); n.second=n.second+std::max(l,u);
      }
    }
    Integer ql=std::min(floor_div(n.first,d.lower),floor_div(n.first,d.upper));
    Integer qu=std::max(floor_div(n.second,d.lower),floor_div(n.second,d.upper));
    if (!q.range(ql,qu) ||
        !inverse_product(q,{n.first-r.upper,n.second-r.lower},{d.lower,d.upper}) ||
        !inverse_product(d,{n.first-r.upper,n.second-r.lower},{q.lower,q.upper})) return false;
    if (q.upper>=zero && !d.range(floor_div(n.first,q.upper+one)+one,d.upper)) return false;
    if (q.lower>zero && !d.range(d.lower,floor_div(n.second,q.lower))) return false;
    auto dq=multiply({d.lower,d.upper},{q.lower,q.upper});
    Integer l=n.first-dq.second;
    Integer u=std::min(d.upper-one,n.second-dq.first);
    if (!r.range(l,u)) return false;
    l=dq.first+r.lower;
    u=dq.second+r.upper;
    if (s.weighted) {
      LinearRow row; row.a=s.weights;
      row.lower=l-s.constant; row.upper=u-s.constant;
      if (!linear_row(row,x,use_knapsack)) return false;
    } else if (!product_filter(s.left,one,x,l,u)) return false;
    if (!s.weighted) {
      // Couple fixed low bits across the whole N=d*q+r identity.
      const std::vector<int> dq={s.divisor,s.quotient},rv={s.remainder};
      auto np=product_prefix(s.left,one,x),dp=product_prefix(dq,one,x),
        rp=product_prefix(rv,one,x);
      if (!congruence(r,one,np.first-dp.first,std::min(np.second,dp.second)) ||
          !product_residue(s.left,one,x,
            {dp.first+rp.first,std::min(dp.second,rp.second)}) ||
          !product_residue(dq,one,x,
            {np.first-rp.first,std::min(np.second,rp.second)})) return false;
    }
    // Constant divisor and remainder constrain every remaining unfixed factor.
    if (d.assigned() && r.assigned()) {
      std::pair<Integer,Integer> target(Integer(r.lower),Integer(d.lower));
      if (s.weighted) {
        LinearRow row; row.a=s.weights; row.lower=row.upper=target.first-s.constant;
        row.modulus=target.second;
        return linear_row(row,x,use_knapsack);
      }
      return product_residue(s.left,one,x,target);
    }
    return true;
  }

  bool linear_row(const LinearRow& row,Domains& x,bool use_knapsack) {
    const size_t n=x.size();
    std::vector<Integer> l(n),u(n),a=row.a;
    Integer lower,upper,constant;
    // Fold assigned variables before the gcd tests.
    for (size_t i=0;i<n;i++) {
      l[i]=a[i]*Integer(x[i].lower); u[i]=a[i]*Integer(x[i].upper);
      if (a[i].neg()) std::swap(l[i],u[i]);
      lower=lower+l[i]; upper=upper+u[i];
      if (x[i].assigned()) { constant=constant+l[i]; a[i]=zero; }
    }
    if (row.modulus.zero()) {
      if (lower>row.upper || upper<row.lower) return false;
      for (size_t i=0;i<n;i++) if (!a[i].zero()) {
        Integer lo=row.lower-(upper-u[i]), hi=row.upper-(lower-l[i]);
        if (a[i].neg()) std::swap(lo,hi);
        if (!x[i].range(ceil_div(lo,a[i]),floor_div(hi,a[i]))) return false;
      }
    }
    if (row.modulus.zero() && row.lower!=row.upper) return !use_knapsack || small_knapsack(row,x);
    Integer rhs=row.lower-constant, g=row.modulus;
    for (const auto& v : a) g=gcd(g,v);
    if (g.zero()) return rhs.zero();
    if (!mod(rhs,g).zero()) return false;
    std::vector<Integer> suffix(n+1);
    suffix[n]=row.modulus;
    for (size_t i=n;i--;) suffix[i]=gcd(a[i],suffix[i+1]);
    Integer prefix=row.modulus;
    for (size_t i=0;i<n;i++) {
      Integer h=gcd(prefix,suffix[i+1]);
      prefix=gcd(prefix,a[i]);
      if (a[i].zero()) continue;
      if (h.zero()) {
        if (!(rhs%a[i]).zero() || !x[i].range(rhs/a[i],rhs/a[i])) return false;
      } else if (!congruence(x[i],a[i],rhs,h)) return false;
    }
    // Known low bits strengthen the row's divisibility condition. All unknown
    // tails are relaxed to arbitrary integers, so this cannot remove a solution.
    Integer tail_g=row.modulus, residue=row.lower;
    for (size_t i=0;i<n;i++) {
      residue=residue-row.a[i]*x[i].base();
      if (!x[i].assigned()) {
        Integer step=Integer::power_two(x[i].prefix());
        tail_g=gcd(tail_g,row.a[i]*step);
      }
    }
    if (!(tail_g.zero() ? residue.zero() : mod(residue,tail_g).zero())) return false;
    return !use_knapsack || small_knapsack(row,x);
  }
  bool linear(const Specification& s,Domains& x,bool use_knapsack) {
    for (const auto& cap : s.caps)
      if (!x[cap.first].range(zero,Integer(cap.second))) return false;
    for (const auto& f : s.fields) {
      Domain& digit=x[f.digit]; Domain& rank=x[f.rank];
      if (!rank.range(zero,rank.upper)) return false;
      // Bits beyond the rank width are zero under exact, nonnegative arithmetic.
      for (unsigned int k=0;k<f.bits;k++) {
        WordValue db=WordValue(1)<<k;
        if (f.shift>=64 || k>=64-f.shift) {
          digit.hi&=~db;
        } else {
          WordValue rb=WordValue(1)<<(f.shift+k);
          if (rank.lo&rb) digit.lo|=db;
          if (!(rank.hi&rb)) digit.hi&=~db;
          if (digit.lo&db) rank.lo|=rb;
          if (!(digit.hi&db)) rank.hi&=~rb;
        }
      }
      if (!digit.sync() || !rank.sync()) return false;
    }
    for (const auto& row : s.rows) if (!linear_row(row,x,false)) return false;
    if (use_knapsack) {
      for (const auto& row : s.rows) if (!small_knapsack(row,x)) return false;
      for (const auto& row : s.derived_rows) if (!linear_row(row,x,true)) return false;
    }
    return true;
  }

  Integer eval_product(const std::vector<int>& f,Integer c,
                       const std::vector<Integer>& x) {
    for (int i : f) c=c*Integer(x[i]);
    return c;
  }
  bool satisfies(const Specification& s,const std::vector<Integer>& x) {
    if (s.family==PRODUCT)
      return eval_product(s.left,s.c,x)==eval_product(s.right,s.d,x);
    if (s.family==DIVMOD) {
      Integer n=s.weighted ? s.constant : eval_product(s.left,one,x);
      if (s.weighted)
        for (size_t i=0;i<x.size();i++) n=n+s.weights[i]*Integer(x[i]);
      return x[s.divisor]>zero && x[s.remainder]>=zero && x[s.remainder]<x[s.divisor] &&
        n==Integer(x[s.divisor])*Integer(x[s.quotient])+Integer(x[s.remainder]);
    }
    for (const auto& cap : s.caps)
      if (x[cap.first]<zero || x[cap.first]>Integer(cap.second)) return false;
    for (const auto& row : s.rows) {
      Integer v;
      for (size_t i=0;i<x.size();i++) v=v+row.a[i]*Integer(x[i]);
      if (row.modulus.zero()) {
        if (v<row.lower || v>row.upper) return false;
      } else if (!mod(v-row.lower,row.modulus).zero()) return false;
    }
    return true;
  }

}}}}

#endif

// STATISTICS: word-prop
