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

#include <gecode/word.hh>
#include <gecode/word/arithmetic/global-propagator.hpp>
#include <cstdint>
#include <limits>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {
  // Fold aliases into columns before arithmetic filtering and row elimination.
  struct Builder {
    WordVarArgs variables;
    /// Open-addressed table of column+1 by var-imp identity, 0 when empty.
    /// Its size is a power of two at least twice the number of columns.
    Array<int> slots;
    static size_t slot(const WordVarImp* x,size_t mask) {
      std::uint64_t h=static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(x)>>4);
      h*=UINT64_C(0x9e3779b97f4a7c15);
      return static_cast<size_t>(h^(h>>32))&mask;
    }
    /// Return the slot holding \a x, or the empty slot where it belongs
    size_t find(const WordVarImp* x) const {
      const size_t mask=slots.size()-1;
      size_t i=slot(x,mask);
      while (slots[i]!=0 && variables[slots[i]-1].varimp()!=x)
        i=(i+1)&mask;
      return i;
    }
    void grow(void) {
      slots=Array<int>(std::max<size_t>(16,2*slots.size()));
      for (int i=0;i<variables.size();i++)
        slots[find(variables[i].varimp())]=i+1;
    }
    int add(WordVar x) {
      if (2*(static_cast<size_t>(variables.size())+1)>slots.size()) grow();
      size_t i=find(x.varimp());
      if (slots[i]!=0) return slots[i]-1;
      if (variables.size()==std::numeric_limits<int>::max())
        throw OutOfLimits("Word::arithmetic globals: arity");
      variables << x; slots[i]=variables.size();
      return variables.size()-1;
    }
    Array<int> add(const WordVarArgs& x) {
      Array<int> v(x.size());
      for (int i=0;i<x.size();i++) v[i]=add(x[i]);
      return v;
    }
    static bool is_equality(const LinearRow& r) {
      return r.modulus.zero() && r.lower==r.upper;
    }
    /// Whether \a a and \a b both have a coefficient in a column before \a k
    static bool share_column_before(const LinearRow& a,const LinearRow& b,size_t k) {
      for (size_t c=0;c<k;c++) if (!a.a[c].zero() && !b.a[c].zero()) return true;
      return false;
    }
    /// Add \a n to \a work, saturating at the largest unsigned int
    static void add_work(unsigned int& work,size_t n) {
      const size_t maximum=std::numeric_limits<unsigned int>::max();
      work=static_cast<unsigned int>(work+std::min(n,maximum-work));
    }
    /// Pair equality rows by bounded elimination; false if a pair contradicts
    bool eliminate(Specification& s) const {
      // Exact integer elimination of two rows sharing a column. A row without
      // coefficients and a nonzero constant refutes the system at every
      // propagation level. Other derived rows are consequences, not
      // replacements for the original rows, and are kept only for advanced
      // propagation, which filters them in its stage.
      // Only equality rows sharing a column can yield an elimination. Index
      // them once, so congruence/range rows and disjoint rows are never paired.
      // The rows of column k are rows[start[k]..start[k+1]), in row order.
      const size_t n=static_cast<size_t>(variables.size());
      Region region;
      size_t* start=region.alloc<size_t>(n+1);
      for (const LinearRow& r : s.rows) if (is_equality(r))
        for (size_t k=0;k<n;k++) if (!r.a[k].zero()) start[k+1]++;
      for (size_t k=0;k<n;k++) start[k+1]+=start[k];
      size_t* rows=region.alloc<size_t>(start[n]);
      size_t* next=region.alloc<size_t>(n);
      for (size_t k=0;k<n;k++) next[k]=start[k];
      for (size_t i=0;i<s.rows.size();i++) if (is_equality(s.rows[i]))
        for (size_t k=0;k<n;k++) if (!s.rows[i].a[k].zero()) rows[next[k]++]=i;
      const size_t derived_limit=32, pair_limit=256;
      size_t tried=0;
      for (size_t k=0;k<n && tried<pair_limit &&
             s.derived_rows.size()<derived_limit;k++) {
        for (size_t i=start[k];i<start[k+1] && tried<pair_limit &&
               s.derived_rows.size()<derived_limit;i++)
          for (size_t j=i+1;j<start[k+1] && tried<pair_limit &&
                 s.derived_rows.size()<derived_limit;j++) {
            // Count attempts too, including repeated pairs from other columns.
            tried++;
            const LinearRow& a=s.rows[rows[i]];
            const LinearRow& b=s.rows[rows[j]];
            // A pair is eliminated once, in the first column both rows share.
            if (share_column_before(a,b,k)) continue;
            Integer g=gcd(a.a[k],b.a[k]), p=b.a[k]/g, q=a.a[k]/g;
            LinearRow r; r.a=Array<Integer>(n);
            bool small=true,nonzero=false;
            for (size_t c=0;c<n;c++) {
              r.a[c]=p*a.a[c]-q*b.a[c];
              small&=r.a[c].bits()<=256; nonzero|=!r.a[c].zero();
            }
            r.lower=r.upper=p*a.lower-q*b.lower;
            if (!nonzero) {
              if (!r.lower.zero()) return false;
              continue;
            }
            if (s.advanced && small && r.lower.bits()<=256)
              s.derived_rows.push_back(r);
          }
      }
      return true;
    }
    void post(Home home,Specification& s) {
      if (!eliminate(s)) { home.fail(); return; }
      // Saturate before addition, including every dense coefficient occurrence.
      add_work(s.work,variables.size()); add_work(s.work,s.left.size());
      add_work(s.work,s.right.size()); add_work(s.work,s.weights.size());
      add_work(s.work,s.caps.size()); add_work(s.work,s.fields.size());
      for (const auto& r : s.rows) { add_work(s.work,1); add_work(s.work,r.a.size()); }
      for (const auto& r : s.derived_rows) { add_work(s.work,1); add_work(s.work,r.a.size()); }
      if (home.failed()) return;
      bool assigned=true;
      for (const auto& v : variables) {
        assigned&=v.assigned();
        s.signed_values.push_back(v.domain_type()==WDT_SIGNED);
      }
      if (assigned) {
        Array<Integer> values;
        for (const auto& v : variables)
          values.push_back(Domain(WordView(v),v.domain_type()==WDT_SIGNED).lower);
        if (!satisfies(s,values)) home.fail();
        return; // Includes zero-arity relations, which have no subscriptions.
      }
      ViewArray<WordView> views(home,variables.size());
      for (int i=0;i<views.size();i++) views[i]=WordView(variables[i]);
      Data data(s);
      switch (s.family) {
      case Family::PRODUCT: Global<Family::PRODUCT>::post(home,views,data); break;
      case Family::DIVMOD: Global<Family::DIVMOD>::post(home,views,data); break;
      case Family::LINEAR: Global<Family::LINEAR>::post(home,views,data); break;
      case Family::RADIX: Global<Family::RADIX>::post(home,views,data); break;
      case Family::IMAGE: Global<Family::IMAGE>::post(home,views,data); break;
      }
    }
  };
}}}}

namespace Gecode {
  using namespace Word::Arithmetic::Global;

  void product_balance(Home home,const WordVarArgs& x,WordValue c,
                       const WordVarArgs& y,WordValue d,IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(Family::PRODUCT,ipl);
    s.left=b.add(x); s.right=b.add(y); s.c=Integer(c); s.d=Integer(d);
    auto left=s.left,right=s.right;
    std::sort(left.begin(),left.end()); std::sort(right.begin(),right.end());
    if (left.size()==right.size() &&
        std::equal(left.begin(),left.end(),right.begin()) && c!=d) {
      // (c-d)*P=0 over integers means P=0. Avoid arbitrarily slow interval
      // convergence when the two coefficients differ by one at full width.
      s.c=one; s.right=Array<int>(); s.d=zero;
    }
    b.post(home,s);
  }
  void product_divmod(Home home,const WordVarArgs& x,WordVar d,WordVar q,
                      WordVar r,IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(Family::DIVMOD,ipl); s.left=b.add(x);
    s.divisor=b.add(d); s.quotient=b.add(q); s.remainder=b.add(r); b.post(home,s);
  }
  void linear_system(Home home,const WordVarArgs& x,
                     const WordLinearRowArgs& rows,
                     IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(Family::LINEAR,ipl); auto indices=b.add(x);
    for (int k=0; k<rows.size(); k++) {
      const WordLinearRow& row=rows[k];
      if (row.coefficients().size()!=static_cast<int>(indices.size()) ||
          (row.type()==WordLinearRow::CONGRUENCE && !row.modulus()) ||
          (row.type()==WordLinearRow::RANGE && row.lower()>row.upper()))
        throw Word::OutOfLimits("Word::linear_system");
      LinearRow r; r.a=Array<Integer>(b.variables.size());
      for (size_t i=0;i<indices.size();i++)
        r.a[indices[i]]=r.a[indices[i]]+
          Integer::signed_value(row.coefficients()[static_cast<int>(i)]);
      r.lower=Integer::signed_value(row.lower());
      r.upper=Integer::signed_value(row.upper());
      if (row.type()==WordLinearRow::CONGRUENCE)
        r.modulus=Integer(row.modulus());
      s.rows.push_back(r);
    }
    b.post(home,s);
  }
  void linear_divmod(Home home,const IntArgs& a,const WordVarArgs& x,int c,
                     WordVar d,WordVar q,WordVar r,IntPropLevel ipl) {
    GECODE_POST;
    if (a.size()!=static_cast<size_t>(x.size()))
      throw Word::OutOfLimits("Word::linear_divmod");
    Builder b; Specification s(Family::DIVMOD,ipl); auto indices=b.add(x);
    s.divisor=b.add(d); s.quotient=b.add(q); s.remainder=b.add(r);
    s.weighted=true; s.constant=Integer::signed_value(c); s.weights=Array<Integer>(b.variables.size());
    for (size_t i=0;i<indices.size();i++)
      s.weights[indices[i]]=s.weights[indices[i]]+Integer::signed_value(a[i]);
    b.post(home,s);
  }
  void quantize_up(Home home,WordVar x,WordValue spacing,WordValue phase,
                   WordVar y,IntPropLevel ipl) {
    GECODE_POST;
    if (!spacing || phase>=spacing)
      throw Word::OutOfLimits("Word::quantize_up");
    Builder b; Specification s(Family::LINEAR,ipl); int in=b.add(x),out=b.add(y);
    LinearRow distance; distance.a=Array<Integer>(b.variables.size());
    distance.a[out]=one; distance.a[in]=distance.a[in]-one;
    distance.upper=Integer(spacing)-one; s.rows.push_back(distance);
    LinearRow grid; grid.a=Array<Integer>(b.variables.size()); grid.a[out]=one;
    grid.lower=grid.upper=Integer(phase); grid.modulus=Integer(spacing);
    s.rows.push_back(grid); b.post(home,s);
  }
  void mixed_radix(Home home,const WordVarArgs& digits,
                   const WordValArgs& radices,WordVar rank,IntPropLevel ipl) {
    GECODE_POST;
    if (radices.size()!=static_cast<size_t>(digits.size()))
      throw Word::OutOfLimits("Word::mixed_radix");
    Builder b; Specification s(Family::RADIX,ipl); auto indices=b.add(digits); int v=b.add(rank);
    LinearRow row; row.a=Array<Integer>(b.variables.size()); row.a[v]=-one;
    Integer stride(1);
    bool power=true; unsigned int shift=0;
    for (size_t i=indices.size();i--;) {
      if (!radices[static_cast<int>(i)])
        throw Word::OutOfLimits("Word::mixed_radix");
      WordValue radix=radices[static_cast<int>(i)];
      s.caps.push_back({indices[i],radix-1});
      bool binary=(radix&(radix-1))==0;
      if (power && binary) {
        unsigned int bits=0;
        for (WordValue r=radix;r>1;r>>=1) bits++;
        s.fields.push_back({indices[i],v,shift,bits});
        shift=std::min(64U,shift+bits);
      }
      power&=binary;
      row.a[indices[i]]=row.a[indices[i]]+stride;
      stride=stride*Integer(radix);
    }
    s.rows.push_back(row); b.post(home,s);
  }
  void bounded_image(Home home,const WordVarArgs& coordinates,
                     const IntArgs& a,const IntArgs& offset,
                     const WordVarArgs& outputs,IntPropLevel ipl) {
    GECODE_POST;
    if (offset.size()!=outputs.size() ||
        static_cast<size_t>(a.size())!=
          static_cast<size_t>(coordinates.size())*static_cast<size_t>(outputs.size()))
      throw Word::OutOfLimits("Word::bounded_image");
    Builder b; Specification s(Family::IMAGE,ipl); auto in=b.add(coordinates),out=b.add(outputs);
    for (int i=0;i<outputs.size();i++) {
      LinearRow row; row.a=Array<Integer>(b.variables.size()); row.a[out[i]]=-one;
      for (size_t j=0;j<in.size();j++)
        row.a[in[j]]=row.a[in[j]]+
          Integer::signed_value(a[static_cast<int>(static_cast<size_t>(i)*in.size()+j)]);
      row.lower=row.upper=-Integer::signed_value(offset[i]);
      s.rows.push_back(row);
    }
    b.post(home,s);
  }
}

// STATISTICS: word-post
