/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include "gecode/minimodel.hh"

#include <algorithm>

namespace Gecode {

  using namespace Int;

  void
  producer_consumer(Space *home,
                    const IntVarArgs& produce_date, const IntArgs& produce_amount,
                    const IntVarArgs& consume_date, const IntArgs& consume_amount,
                    int initial, IntConLevel cl)
  {
    if (produce_date.size() != produce_amount.size() ||
        consume_date.size() != consume_amount.size())
      throw ArgumentSizeMismatch("Minimodel::producer_consumer");

    int ntasks = produce_date.size() + consume_date.size();

    IntArgs machine(ntasks), height(ntasks), limit(1);
    IntVarArgs start(ntasks), duration(ntasks), end(ntasks);

    int i = 0;
    int sum_height = initial;
    Int::Limits::check(initial,"MiniModel::producer_consumer");

    int maxval = 0;
    for (int j = produce_date.size(); j--; )
        maxval = std::max(produce_date[i].max(), maxval);
    for (int j = consume_date.size(); j--; )
        maxval = std::max(consume_date[j].max(), maxval);
    ++maxval;

    IntVar minvar = IntVar(home, 0, 0);
    IntVar maxvar = IntVar(home, maxval, maxval);


    // Construct producer tasks
    for (int k = produce_date.size(); k--; ++i) {
      sum_height += produce_amount[k];
      machine[i] = 0;

      start[i] = minvar;
      end[i] = produce_date[k];
      duration[i] = IntVar(home, end[i].min(), end[i].max());
      height[i] = produce_amount[k];
      Int::Limits::check(height[i],"MiniModel::producer_consumer");
    }

    // Construct consumer tasks
    for (int k = consume_date.size(); k--; ++i) {
      machine[i] = 0;

      start[i] = consume_date[k];
      end[i] = maxvar;
      duration[i] = IntVar(home, maxval - start[i].max(),
                           maxval - start[i].min());
      height[i] = consume_amount[k];
      Int::Limits::check(height[i],"MiniModel::producer_consumer");
    }

    limit[0] = sum_height;

    cumulatives(home, machine, start, duration, end, height, limit, true, cl);
  }


  namespace {
    /// Delayed optimized construction of constant value.
    class ConstVar {
      Space *home_;
      int val_;
    public:
      ConstVar(Space *home, int val) : home_(home), val_(val) {}

      operator int() { return val_; }
      operator IntVar() { return IntVar(home_, val_, val_); }
    };

    /// Return an IntVar representing the constant value \a val
    IntVar make_intvar(Space *home, int val)
    {
      return IntVar(home, val, val);
    }

    /// Return the IntVar  \a iv directly
    IntVar make_intvar(Space*, IntVar iv)
    {
      return iv;
    }

    template<class Duration, class Height>
    void
    post_cumulative(Space *home, const IntVarArgs& start, const Duration& duration,
                    const Height& height, int limit, bool at_most, IntConLevel cl)
    {
      if (start.size() != duration.size() ||
          duration.size() !=  height.size())
        throw ArgumentSizeMismatch("MiniModel::cumulative");

      Int::Limits::check(limit, "MiniModel::cumulative");

      int n = start.size() + !at_most;
      IntArgs m(n), l(1, limit);
      IntVarArgs s(n), d(n), e(n);
      Height h(n);

      if (!at_most) {
        int smin = Int::Limits::int_max, 
            smax = Int::Limits::int_min, 
            emin = Int::Limits::int_max, 
            emax = Int::Limits::int_min;
        IntVarArgs end(n-1);
        for (int i = start.size(); i--; ) {
          m[i] = 0;
          s[i] = start[i];
          smin = std::min(s[i].min(), smin);
          smax = std::max(s[i].max(), smax);
          d[i] = make_intvar(home, duration[i]);
          e[i] = IntVar(home, Int::Limits::int_min, Int::Limits::int_max);
          end[i] = e[i];
          emin = std::min(e[i].min(), emin);
          emax = std::max(e[i].max(), emax);
          h[i] = height[i];
        }
        m[n-1] = 0;
        s[n-1] = IntVar(home, smin, smax);
        d[n-1] = IntVar(home, 0, emax - smin);
        e[n-1] = IntVar(home, emin, emax);
        h[n-1] = ConstVar(home, 0);
        
        min(home, start, s[n-1]);
        max(home, end, e[n-1]);
      } else {
        for (int i = start.size(); i--; ) {
          m[i] = 0;
          s[i] = start[i];
          d[i] = make_intvar(home, duration[i]);
          e[i] = IntVar(home, s[i].min()+d[i].min(), s[i].max()+d[i].max());
          h[i] = height[i];
        }
      }

      cumulatives(home, m, s, d, e, h, l, at_most, cl);
    }

  }

  void
  cumulative(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             const IntVarArgs& height, int limit, bool at_most, IntConLevel cl)
  {
    post_cumulative(home, start, duration, height, limit, at_most, cl);
  }

  void
  cumulative(Space *home, const IntVarArgs& start, const IntArgs& duration,
             const IntVarArgs& height, int limit, bool at_most, IntConLevel cl)
  {
    post_cumulative(home, start, duration, height, limit, at_most, cl);
  }

  void
  cumulative(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             const IntArgs& height, int limit, bool at_most, IntConLevel cl)
  {
    post_cumulative(home, start, duration, height, limit, at_most, cl);
  }

  void
  cumulative(Space *home, const IntVarArgs& start, const IntArgs& duration,
             const IntArgs& height, int limit, bool at_most, IntConLevel cl)
  {
    post_cumulative(home, start, duration, height, limit, at_most, cl);
  }


  namespace {
    template <class Duration>
    void
    post_serialized(Space *home, const IntVarArgs& start, const Duration& duration,
               IntConLevel cl)
    {
      if (start.size() != duration.size())
        throw ArgumentSizeMismatch("MiniModel::serialized");

      IntArgs height(start.size());
      for (int i = start.size(); i--; ) height[i] = 1;

      post_cumulative(home, start, duration, height, 1, true, cl);
    }
  }

  void
  serialized(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             IntConLevel cl)
  {
    post_serialized(home, start, duration, cl);
  }


  void
  serialized(Space *home, const IntVarArgs& start, const IntArgs& duration,
             IntConLevel cl)
  {
        post_serialized(home, start, duration, cl);
  }

}

// STATISTICS: minimodel-any
