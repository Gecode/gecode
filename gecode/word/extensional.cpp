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
#include <algorithm>
#include <limits>
#include <numeric>

namespace Gecode {

  class WordTupleSet::Data : public SharedHandle::Object {
  public:
    std::vector<unsigned int> widths;
    std::vector<std::vector<WordValue>> rows;
    // One bitmap per column bit; zero supports are its complement.
    std::vector<std::vector<WordValue>> ones;
    std::vector<int> offset;
    std::vector<std::vector<int>> order;
    int blocks;
    Data(const std::vector<unsigned int>& w,
         const std::vector<std::vector<WordValue>>& r)
      : widths(w), rows(r), blocks(0) {
      if ((w.size() > static_cast<size_t>(std::numeric_limits<int>::max()/128)) ||
          (r.size() > static_cast<size_t>(std::numeric_limits<int>::max()-63)))
        throw Word::OutOfLimits("WordTupleSet");
      int bits=0;
      for (unsigned int width : w) {
        if ((width == 0) || (width > 64))
          throw Word::OutOfLimits("WordTupleSet");
        offset.push_back(bits);
        bits+=width;
      }
      for (const auto& row : rows) {
        if (row.size() != w.size())
          throw Word::OutOfLimits("WordTupleSet");
        for (size_t i=0; i<w.size(); i++)
          if ((w[i] < 64) && (row[i] >> w[i]))
            throw Word::OutOfLimits("WordTupleSet");
      }
      std::sort(rows.begin(),rows.end());
      rows.erase(std::unique(rows.begin(),rows.end()),rows.end());
      blocks=(static_cast<int>(rows.size())+63)/64;
      ones.assign(bits,std::vector<WordValue>(blocks,0));
      order.resize(w.size()*2);
      for (size_t i=0; i<w.size(); i++) {
        for (size_t t=0; t<rows.size(); t++)
          for (unsigned int b=0; b<w[i]; b++)
            if (rows[t][i] & (WordValue(1)<<b))
              ones[offset[i]+b][t/64] |= WordValue(1)<<(t%64);
        auto& unsigned_order=order[2*i];
        unsigned_order.resize(rows.size());
        std::iota(unsigned_order.begin(),unsigned_order.end(),0);
        // Rows are already lexicographically sorted on the first column.
        if (i != 0)
          std::sort(unsigned_order.begin(),unsigned_order.end(),[&](int a, int b) {
            return rows[a][i] < rows[b][i];
          });
        auto& signed_order=order[2*i+1];
        WordValue sign=WordValue(1)<<(w[i]-1);
        auto split=std::lower_bound(unsigned_order.begin(),unsigned_order.end(),sign,
          [&](int row, WordValue value) { return rows[row][i]<value; });
        signed_order.insert(signed_order.end(),split,unsigned_order.end());
        signed_order.insert(signed_order.end(),unsigned_order.begin(),split);
      }
    }
  };

  WordTupleSet::WordTupleSet(const std::vector<unsigned int>& widths,
                            const std::vector<std::vector<WordValue>>& rows)
    : SharedHandle(new Data(widths,rows)) {}
  const WordTupleSet::Data& WordTupleSet::data(void) const {
    return *static_cast<const Data*>(object());
  }
  int WordTupleSet::arity(void) const { return static_cast<int>(data().widths.size()); }
  int WordTupleSet::tuples(void) const { return static_cast<int>(data().rows.size()); }
  unsigned int WordTupleSet::width(int i) const { return data().widths[i]; }
  WordValue WordTupleSet::value(int t, int i) const { return data().rows[t][i]; }

  namespace Word { namespace Extensional {

    class Compact : public Propagator {
    protected:
      ViewArray<WordView> x;
      WordTupleSet table;
      WordValue* live;
      int* index;
      int limit;
      int* residue;
      int* lower;
      int* upper;
      WordValue* previous_lo;
      WordValue* previous_hi;

      bool alive(int t) const {
        return (live[t/64] & (WordValue(1)<<(t%64))) != 0;
      }
      void remove(int t) { live[t/64] &= ~(WordValue(1)<<(t%64)); }
      void compact(void) {
        for (int p=limit; p--;)
          if (!live[index[p]]) index[p]=index[--limit];
      }
      bool witness(int bit, bool one) {
        const auto& support=table.data().ones[bit];
        int& r=residue[2*bit+one];
        if (live[r] & (one ? support[r] : ~support[r])) return true;
        for (int p=0; p<limit; p++) {
          int k=index[p];
          if (live[k] & (one ? support[k] : ~support[k])) {
            r=k; return true;
          }
        }
        return false;
      }
      Compact(Home home, ViewArray<WordView>& v, const WordTupleSet& t)
        : Propagator(home), x(v), table(t), limit(t.data().blocks) {
        Space& space=home;
        const int n=t.data().blocks;
        live=space.alloc<WordValue>(n);
        index=space.alloc<int>(n);
        residue=space.alloc<int>(static_cast<int>(t.data().ones.size())*2);
        lower=space.alloc<int>(x.size());
        upper=space.alloc<int>(x.size());
        previous_lo=space.alloc<WordValue>(x.size());
        previous_hi=space.alloc<WordValue>(x.size());
        for (int k=0; k<n; k++) { live[k]=~WordValue(0); index[k]=k; }
        if (t.tuples()%64) live[n-1]=(WordValue(1)<<(t.tuples()%64))-1;
        std::fill(residue,residue+t.data().ones.size()*2,0);
        for (int i=0; i<x.size(); i++) {
          lower[i]=0; upper[i]=t.tuples()-1;
          previous_lo[i]=0; previous_hi[i]=x[i].mask();
          // An aliased column must agree within each individual tuple.
          for (int j=0; j<i; j++)
            if (x[i] == x[j])
              for (int r=0; r<t.tuples(); r++)
                if (t.value(r,i) != t.value(r,j)) remove(r);
        }
        x.subscribe(home,*this,PC_WORD_DOM);
        home.notice(*this,AP_DISPOSE);
      }
      Compact(Space& home, Compact& p)
        : Propagator(home,p), table(p.table), limit(p.limit) {
        x.update(home,p.x);
        int n=table.data().blocks;
        live=home.alloc<WordValue>(n);
        std::copy(p.live,p.live+n,live);
        index=home.alloc<int>(limit);
        std::copy(p.index,p.index+limit,index);
        int nr=static_cast<int>(table.data().ones.size())*2;
        residue=home.alloc<int>(nr);
        std::copy(p.residue,p.residue+nr,residue);
        lower=home.alloc<int>(x.size()); upper=home.alloc<int>(x.size());
        previous_lo=home.alloc<WordValue>(x.size());
        previous_hi=home.alloc<WordValue>(x.size());
        std::copy(p.lower,p.lower+x.size(),lower);
        std::copy(p.upper,p.upper+x.size(),upper);
        std::copy(p.previous_lo,p.previous_lo+x.size(),previous_lo);
        std::copy(p.previous_hi,p.previous_hi+x.size(),previous_hi);
      }
    public:
      static void post(Home home, ViewArray<WordView>& x, const WordTupleSet& t) {
        (void) new (home) Compact(home,x,t);
      }
      virtual Actor* copy(Space& home) { return new (home) Compact(home,*this); }
      virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::HI,x.size());
      }
      virtual void reschedule(Space& home) { x.reschedule(home,*this,PC_WORD_DOM); }
      virtual size_t dispose(Space& home) {
        home.ignore(*this,AP_DISPOSE);
        x.cancel(home,*this,PC_WORD_DOM);
        table.~WordTupleSet();
        (void) Propagator::dispose(home);
        return sizeof(*this);
      }
      virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        const auto& d=table.data();
        // Only newly fixed bits need to intersect the reversible sparse table.
        for (int i=0; i<x.size(); i++) {
          WordValue changed=(x[i].lo()^previous_lo[i]) |
                            (x[i].hi()^previous_hi[i]);
          for (unsigned int b=0; b<x[i].width(); b++) {
            WordValue mask=WordValue(1)<<b;
            if (!(changed & mask)) continue;
            const auto& support=d.ones[d.offset[i]+b];
            bool one=(x[i].lo() & mask) != 0;
            for (int p=limit; p--;) {
              int k=index[p];
              live[k] &= one ? support[k] : ~support[k];
              if (!live[k]) index[p]=index[--limit];
            }
          }
          if (x[i].bounded()) {
            const auto& o=d.order[2*i+(x[i].domain_type()==WDT_SIGNED)];
            WordValue lo=x[i].rank_minimum(), hi=x[i].rank_maximum();
            while (lower[i]<=upper[i] &&
                   rank(x[i].domain_type(),x[i].width(),d.rows[o[lower[i]]][i])<lo)
              remove(o[lower[i]++]);
            while (lower[i]<=upper[i] &&
                   rank(x[i].domain_type(),x[i].width(),d.rows[o[upper[i]]][i])>hi)
              remove(o[upper[i]--]);
          }
        }
        compact();
        if (!limit) return ES_FAILED;
        bool assigned=true;
        for (int i=0; i<x.size(); i++) {
          WordValue lo=x[i].lo(), hi=x[i].hi(), unknown=x[i].unknown();
          for (unsigned int b=0; b<x[i].width(); b++) {
            WordValue mask=WordValue(1)<<b;
            if (!(unknown & mask)) continue;
            if (!witness(d.offset[i]+b,false)) lo |= mask;
            if (!witness(d.offset[i]+b,true)) hi &= ~mask;
          }
          GECODE_ME_CHECK(x[i].narrow(home,lo,hi));
          if (x[i].bounded()) {
            const auto& o=d.order[2*i+(x[i].domain_type()==WDT_SIGNED)];
            while (!alive(o[lower[i]])) lower[i]++;
            while (!alive(o[upper[i]])) upper[i]--;
            GECODE_ME_CHECK(x[i].narrow_rank_range(home,
              rank(x[i].domain_type(),x[i].width(),d.rows[o[lower[i]]][i]),
              rank(x[i].domain_type(),x[i].width(),d.rows[o[upper[i]]][i])));
          }
          previous_lo[i]=x[i].lo(); previous_hi[i]=x[i].hi();
          assigned=assigned && x[i].assigned();
        }
        // Every remaining tuple survives its own hull, including aliases.
        return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    };
  }}

  void extensional(Home home, const WordVarArgs& x, const WordTupleSet& t) {
    if (x.size() != t.arity()) throw Word::OutOfLimits("Word::extensional");
    for (int i=0; i<x.size(); i++)
      if (x[i].width() != t.width(i))
        throw Word::WidthMismatch("Word::extensional");
    GECODE_POST;
    if (!t.tuples()) { home.fail(); return; }
    if (!x.size()) return;
    ViewArray<Word::WordView> views(home,x);
    Word::Extensional::Compact::post(home,views,t);
  }
}

// STATISTICS: word-prop
