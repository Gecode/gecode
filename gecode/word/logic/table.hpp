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
    for (unsigned int t=0; t<tuples; t++) {
      allowed_[t] = a[t];
    }
  }

  forceinline
  Table::Table(Space& home, Table& p)
    : NaryPropagator<WordView,PC_WORD_BITS>(home,p) {
    const unsigned int tuples = 1U << x.size();
    for (unsigned int t=0; t<tuples; t++) {
      allowed_[t] = p.allowed_[t];
    }
  }

  forceinline Actor*
  Table::copy(Space& home) {
    return new (home) Table(home,*this);
  }

  forceinline size_t
  Table::dispose(Space& home) {
    (void) NaryPropagator<WordView,PC_WORD_BITS>::dispose(home);
    return sizeof(*this);
  }

  forceinline PropCost
  Table::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  /// Fixed-size local cubes for a truth table of n distinct views.
  template<int n>
  struct TableLogicMasks {
    WordValue lo[n], hi[n];
  };

  /// Complete local table closure, before any view is narrowed.
  template<int n>
  struct TableLogicResult {
    bool has_support;
    TableLogicMasks<n> masks;
  };

  /// Computes tuple support to a local fixed point, with no Home effects.
  template<int n>
  forceinline TableLogicResult<n>
  close_table_masks(TableLogicMasks<n> masks, WordValue mask,
                    const WordValue* allowed) {
    bool has_changed;
    do {
      WordValue support[n][2]={};
      const unsigned int tuples=1U << n;
      for (unsigned int t=0; t<tuples; t++) {
        if (allowed[t] == 0)
          continue;
        WordValue tuple_support=allowed[t];
        for (int i=0; i<n; i++) {
          tuple_support &= ((t & (1U << i)) != 0)
            ? masks.hi[i] : (~masks.lo[i] & mask);
        }
        for (int i=0; i<n; i++) {
          support[i][(t >> i) & 1U] |= tuple_support;
        }
      }
      has_changed=false;
      for (int i=0; i<n; i++) {
        const WordValue next_lo=masks.lo[i] | (~support[i][0] & mask);
        const WordValue next_hi=masks.hi[i] & support[i][1];
        const bool has_support=(next_lo & ~next_hi) == 0;
        if (!has_support)
          return {false,masks};
        has_changed |= (next_lo != masks.lo[i]) || (next_hi != masks.hi[i]);
        masks.lo[i]=next_lo;
        masks.hi[i]=next_hi;
      }
    } while (has_changed);
    return {true,masks};
  }

  /// Publishes completed local table masks in view order, repeating feedback.
  template<int n>
  forceinline ExecStatus
  narrow_table(Home home, ViewArray<WordView>& x, const WordValue* allowed) {
    const WordValue mask=x[0].mask();
    bool has_bounded_view=false;
    for (int i=0; i<n; i++) {
      has_bounded_view |= x[i].bounded();
    }
    for (;;) {
      TableLogicMasks<n> before;
      for (int i=0; i<n; i++) {
        before.lo[i]=x[i].lo();
        before.hi[i]=x[i].hi();
      }
      const TableLogicResult<n> result=close_table_masks(before,mask,allowed);
      if (!result.has_support)
        return ES_FAILED;
      const TableLogicMasks<n>& masks=result.masks;
      for (int i=0; i<n; i++) {
        const bool has_change=(masks.lo[i] != x[i].lo()) ||
          (masks.hi[i] != x[i].hi());
        if (has_change)
          GECODE_ME_CHECK(x[i].narrow(home,masks.lo[i],masks.hi[i]));
      }
      if (!has_bounded_view)
        return ES_OK;
      bool is_synchronized=true;
      for (int i=0; i<n; i++) {
        is_synchronized &= (masks.lo[i] == x[i].lo()) &&
          (masks.hi[i] == x[i].hi());
      }
      if (is_synchronized)
        return ES_OK;
    }
  }

  forceinline ExecStatus
  Table::narrow(Home home, ViewArray<WordView>& x,
                const WordValue* allowed) {
    assert((x.size() >= 1) && (x.size() <= 4));
    switch (x.size()) {
    case 1: return narrow_table<1>(home,x,allowed);
    case 2: return narrow_table<2>(home,x,allowed);
    case 3: return narrow_table<3>(home,x,allowed);
    case 4: return narrow_table<4>(home,x,allowed);
    default: GECODE_NEVER;
    }
    return ES_FAILED;
  }

  forceinline ExecStatus
  Table::post(Home home, ViewArray<WordView>& x, const WordValue* allowed) {
    GECODE_ES_CHECK(narrow(home,x,allowed));
    bool is_assigned=true;
    for (int i=0; i<x.size(); i++) {
      is_assigned &= x[i].assigned();
    }
    if (!is_assigned)
      (void) new (home) Table(home,x,allowed);
    return ES_OK;
  }

  forceinline ExecStatus
  Table::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x,allowed_));
    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned())
        return ES_FIX;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
