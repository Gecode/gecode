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
#include <limits>
#include <set>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {
  // Fold aliases into columns before arithmetic filtering and row elimination.
  struct Builder {
    std::vector<WordVar> variables;
    int add(WordVar x) {
      for (size_t i=0;i<variables.size();i++)
        if (variables[i].varimp()==x.varimp()) return static_cast<int>(i);
      if (variables.size()>=static_cast<size_t>(std::numeric_limits<int>::max()))
        throw OutOfLimits("Word::arithmetic globals: arity");
      variables.push_back(x); return static_cast<int>(variables.size()-1);
    }
    std::vector<int> add(const WordVarArgs& x) {
      std::vector<int> v;
      for (int i=0;i<x.size();i++) v.push_back(add(x[i]));
      return v;
    }
    void post(Home home,Specification& s) {
      // Bounded pairwise integer elimination, preserving exact coefficients.
      // Derived rows are consequences, not replacements for the original rows.
      // They are built only for advanced propagation and filtered in its stage.
      if (s.advanced) {
        // Only equality rows sharing a column can yield an elimination. Index
        // them once, so congruence/range rows and disjoint rows are never paired.
        std::vector<std::vector<size_t>> columns(variables.size());
        for (size_t i=0;i<s.rows.size();i++) {
          const LinearRow& r=s.rows[i];
          if (!r.modulus.zero() || r.lower!=r.upper) continue;
          for (size_t k=0;k<variables.size();k++)
            if (!r.a[k].zero()) columns[k].push_back(i);
        }
        const size_t derived_limit=32, pair_limit=256;
        size_t tried=0;
        std::set<std::pair<size_t,size_t>> seen;
        for (size_t k=0;k<columns.size() && tried<pair_limit &&
               s.derived_rows.size()<derived_limit;k++) {
          const auto& rows=columns[k];
          for (size_t i=0;i<rows.size() && tried<pair_limit &&
                 s.derived_rows.size()<derived_limit;i++)
            for (size_t j=i+1;j<rows.size() && tried<pair_limit &&
                   s.derived_rows.size()<derived_limit;j++) {
              // Count attempts too, including repeated pairs from other columns.
              tried++;
              if (!seen.insert({rows[i],rows[j]}).second) continue;
              const LinearRow& a=s.rows[rows[i]];
              const LinearRow& b=s.rows[rows[j]];
              Integer g=gcd(a.a[k],b.a[k]), p=b.a[k]/g, q=a.a[k]/g;
              LinearRow r; r.a.resize(variables.size());
              bool small=true,nonzero=false;
              for (size_t c=0;c<variables.size();c++) {
                r.a[c]=p*a.a[c]-q*b.a[c];
                small&=r.a[c].bits()<=256; nonzero|=!r.a[c].zero();
              }
              r.lower=r.upper=p*a.lower-q*b.lower;
              if (!nonzero) {
                if (!r.lower.zero()) { home.fail(); return; }
                continue;
              }
              if (small && r.lower.bits()<=256) s.derived_rows.push_back(r);
            }
        }
      }
      // Saturate before addition, including every dense coefficient occurrence.
      const size_t maximum=std::numeric_limits<unsigned int>::max();
      auto count=[&](size_t n) {
        s.work=static_cast<unsigned int>(s.work+std::min(n,maximum-s.work));
      };
      count(variables.size()); count(s.left.size()); count(s.right.size());
      count(s.weights.size()); count(s.caps.size()); count(s.fields.size());
      for (const auto& r : s.rows) { count(1); count(r.a.size()); }
      for (const auto& r : s.derived_rows) { count(1); count(r.a.size()); }
      if (home.failed()) return;
      bool assigned=true;
      for (const auto& v : variables) {
        assigned&=v.assigned();
        s.signed_values.push_back(v.domain_type()==WDT_SIGNED);
      }
      if (assigned) {
        std::vector<Integer> values;
        for (const auto& v : variables)
          values.push_back(Domain(WordView(v),v.domain_type()==WDT_SIGNED).lower);
        if (!satisfies(s,values)) home.fail();
        return; // Includes zero-arity relations, which have no subscriptions.
      }
      ViewArray<WordView> views(home,static_cast<int>(variables.size()));
      for (int i=0;i<views.size();i++) views[i]=WordView(variables[i]);
      Data data(s);
      switch (s.family) {
      case PRODUCT: Global<PRODUCT>::post(home,views,data); break;
      case DIVMOD: Global<DIVMOD>::post(home,views,data); break;
      case LINEAR: Global<LINEAR>::post(home,views,data); break;
      case RADIX: Global<RADIX>::post(home,views,data); break;
      case IMAGE: Global<IMAGE>::post(home,views,data); break;
      }
    }
  };
}}}}

namespace Gecode {
  using namespace Word::Arithmetic::Global;

  void product_balance(Home home,const WordVarArgs& x,WordValue c,
                       const WordVarArgs& y,WordValue d,IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(PRODUCT,ipl);
    s.left=b.add(x); s.right=b.add(y); s.c=Integer(c); s.d=Integer(d);
    auto left=s.left,right=s.right;
    std::sort(left.begin(),left.end()); std::sort(right.begin(),right.end());
    if (left==right && c!=d) {
      // (c-d)*P=0 over integers means P=0. Avoid arbitrarily slow interval
      // convergence when the two coefficients differ by one at full width.
      s.c=one; s.right.clear(); s.d=zero;
    }
    b.post(home,s);
  }
  void product_divmod(Home home,const WordVarArgs& x,WordVar d,WordVar q,
                      WordVar r,IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(DIVMOD,ipl); s.left=b.add(x);
    s.divisor=b.add(d); s.quotient=b.add(q); s.remainder=b.add(r); b.post(home,s);
  }
  void linear_system(Home home,const WordVarArgs& x,
                     const WordLinearRowArgs& rows,
                     IntPropLevel ipl) {
    GECODE_POST;
    Builder b; Specification s(LINEAR,ipl); auto indices=b.add(x);
    for (int k=0; k<rows.size(); k++) {
      const WordLinearRow& row=rows[k];
      if (row.coefficients().size()!=static_cast<int>(indices.size()) ||
          (row.type()==WordLinearRow::CONGRUENCE && !row.modulus()) ||
          (row.type()==WordLinearRow::RANGE && row.lower()>row.upper()))
        throw Word::OutOfLimits("Word::linear_system");
      LinearRow r; r.a.resize(b.variables.size());
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
    Builder b; Specification s(DIVMOD,ipl); auto indices=b.add(x);
    s.divisor=b.add(d); s.quotient=b.add(q); s.remainder=b.add(r);
    s.weighted=true; s.constant=Integer::signed_value(c); s.weights.resize(b.variables.size());
    for (size_t i=0;i<indices.size();i++)
      s.weights[indices[i]]=s.weights[indices[i]]+Integer::signed_value(a[i]);
    b.post(home,s);
  }
  void quantize_up(Home home,WordVar x,WordValue spacing,WordValue phase,
                   WordVar y,IntPropLevel ipl) {
    GECODE_POST;
    if (!spacing || phase>=spacing)
      throw Word::OutOfLimits("Word::quantize_up");
    Builder b; Specification s(LINEAR,ipl); int in=b.add(x),out=b.add(y);
    LinearRow distance; distance.a.resize(b.variables.size());
    distance.a[out]=one; distance.a[in]=distance.a[in]-one;
    distance.upper=Integer(spacing)-one; s.rows.push_back(distance);
    LinearRow grid; grid.a.resize(b.variables.size()); grid.a[out]=one;
    grid.lower=grid.upper=Integer(phase); grid.modulus=Integer(spacing);
    s.rows.push_back(grid); b.post(home,s);
  }
  void mixed_radix(Home home,const WordVarArgs& digits,
                   const WordValArgs& radices,WordVar rank,IntPropLevel ipl) {
    GECODE_POST;
    if (radices.size()!=static_cast<size_t>(digits.size()))
      throw Word::OutOfLimits("Word::mixed_radix");
    Builder b; Specification s(RADIX,ipl); auto indices=b.add(digits); int v=b.add(rank);
    LinearRow row; row.a.resize(b.variables.size()); row.a[v]=-one;
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
        a.size()!=coordinates.size()*outputs.size())
      throw Word::OutOfLimits("Word::bounded_image");
    Builder b; Specification s(IMAGE,ipl); auto in=b.add(coordinates),out=b.add(outputs);
    for (int i=0;i<outputs.size();i++) {
      LinearRow row; row.a.resize(b.variables.size()); row.a[out[i]]=-one;
      for (size_t j=0;j<in.size();j++)
        row.a[in[j]]=row.a[in[j]]+
          Integer::signed_value(a[i*coordinates.size()+static_cast<int>(j)]);
      row.lower=row.upper=-Integer::signed_value(offset[i]);
      s.rows.push_back(row);
    }
    b.post(home,s);
  }
}

// STATISTICS: word-post
