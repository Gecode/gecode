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

namespace Gecode { namespace Word { namespace Logic {

  forceinline
  Table::Table(Home home, ViewArray<WordView>& y, const WordValue* a)
    : NaryPropagator<WordView,PC_WORD_BITS>(home,y) {
    const unsigned int tuples = 1U << y.size();
    for (unsigned int t=0; t<tuples; t++)
      allowed[t] = a[t];
  }

  forceinline
  Table::Table(Space& home, Table& p)
    : NaryPropagator<WordView,PC_WORD_BITS>(home,p) {
    const unsigned int tuples = 1U << x.size();
    for (unsigned int t=0; t<tuples; t++)
      allowed[t] = p.allowed[t];
  }

  forceinline Actor*
  Table::copy(Space& home) {
    return new (home) Table(home,*this);
  }

  forceinline PropCost
  Table::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  template<int n>
  forceinline ExecStatus
  table_narrow(Home home, ViewArray<WordView>& x,
               const WordValue* allowed) {
    const WordValue mask = x[0].mask();
    WordValue lo[n], hi[n];
    bool bounded=false;
    for (int i=0; i<n; i++)
      bounded |= x[i].bounded();
    for (;;) {
      for (int i=0; i<n; i++) {
        lo[i] = x[i].lo();
        hi[i] = x[i].hi();
      }
      bool changed;
      do {
        WordValue support[n][2] = {};
        const unsigned int tuples = 1U << n;
        for (unsigned int t=0; t<tuples; t++) {
          if (allowed[t] == 0)
            continue;
          WordValue tuple_support = allowed[t];
          for (int i=0; i<n; i++)
            tuple_support &= ((t & (1U << i)) != 0)
              ? hi[i] : (~lo[i] & mask);
          for (int i=0; i<n; i++)
            support[i][(t >> i) & 1U] |= tuple_support;
        }
        changed = false;
        for (int i=0; i<n; i++) {
          const WordValue next_lo = lo[i] | (~support[i][0] & mask);
          const WordValue next_hi = hi[i] & support[i][1];
          if ((next_lo & ~next_hi) != 0)
            return ES_FAILED;
          changed |= (next_lo != lo[i]) || (next_hi != hi[i]);
          lo[i] = next_lo;
          hi[i] = next_hi;
        }
      } while (changed);
      for (int i=0; i<n; i++) {
        if ((lo[i] != x[i].lo()) || (hi[i] != x[i].hi()))
          GECODE_ME_CHECK(x[i].narrow(home,lo[i],hi[i]));
      }
      if (!bounded)
        return ES_OK;
      bool synchronized=true;
      for (int i=0; i<n; i++)
        synchronized &= (lo[i] == x[i].lo()) && (hi[i] == x[i].hi());
      if (synchronized)
        return ES_OK;
    }
  }

  forceinline ExecStatus
  Table::narrow(Home home, ViewArray<WordView>& x,
                const WordValue* allowed) {
    assert((x.size() >= 1) && (x.size() <= 4));
    switch (x.size()) {
    case 1: return table_narrow<1>(home,x,allowed);
    case 2: return table_narrow<2>(home,x,allowed);
    case 3: return table_narrow<3>(home,x,allowed);
    case 4: return table_narrow<4>(home,x,allowed);
    default: GECODE_NEVER;
    }
    return ES_FAILED;
  }

  forceinline ExecStatus
  Table::post(Home home, ViewArray<WordView>& x, const WordValue* allowed) {
    GECODE_ES_CHECK(narrow(home,x,allowed));
    bool assigned = true;
    for (int i=0; i<x.size(); i++)
      assigned &= x[i].assigned();
    if (!assigned)
      (void) new (home) Table(home,x,allowed);
    return ES_OK;
  }

  forceinline ExecStatus
  Table::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x,allowed));
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned())
        return ES_FIX;
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
