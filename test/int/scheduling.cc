/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"

#include "gecode/minimodel.hh"
#include "gecode/search.hh"

#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

namespace Test { namespace Int { namespace Scheduling {

using namespace Gecode;

static Gecode::IntSet ds_12(-1,2);

namespace {
  using namespace Gecode::Iter::Ranges;

  /* We are only interested in assignments that represent tasks (s_i,
   * d_i, e_i, h_i) such that the following hold:
   *   - The task starts at a positive time and has some extension.
   *   - The equation s_i + d_i = e_i holds.
   *   - The tasks are ordered to remove some symmetries, i.e.,
   *     s_i <= s_{i+1}
   */
  struct Ass : public Gecode::Space {
    Gecode::IntVarArray x;
    Ass(int n, const Gecode::IntSet& d) : x(this, n, d) {
      for (int i = 0; i < n; i += 4) {
        /// Post constraint on \a x
        post(this, x[i+0] >= 0);
        /// Post constraint on \a x
        post(this, x[i+1] >= 0);
        /// Post constraint on \a x
        post(this, x[i+2] >= 0);
        /// Post constraint on \a x
        post(this, x[i] + x[i+1] == x[i+2]);
        // The following doesn't work with search-based tests
        //if (i+4 < n) {
        /// Post constraint on \a x
        //   post(this, x[i] <= x[i+4]);
        //}
        branch(this, x, INT_VAR_NONE, INT_VAL_MIN);
      }
    }
    Ass(bool share, Ass& s) : Gecode::Space(share,s) {
      x.update(this, share, s.x);
    }
    virtual Gecode::Space* copy(bool share) {
      return new Ass(share,*this);
    }
  };

  // Class for generating reasonable assignements (i.e. s+d=e)
  class CumulativeAssignment : public Assignment {
    Ass *cur, *nxt;
    DFS<Ass> *e;
  public:
    /// Create and register test
    CumulativeAssignment(int, const Gecode::IntSet&);
    virtual void reset(void);
    virtual void operator++(void);
    virtual int  operator[](int i) const;
    virtual bool operator()(void)  const;
    virtual ~CumulativeAssignment() {
      if (cur) delete cur;
      if (nxt) delete nxt;
      if (e  ) delete e;
    }
  };

  CumulativeAssignment::CumulativeAssignment(int n0, const Gecode::IntSet& d0)
    : Assignment(n0, d0), cur(NULL), nxt(NULL), e(NULL)
  {
    reset();
  }
  void
  CumulativeAssignment::reset(void) {
    Ass *a = new Ass(n, d);
    delete e;
    e = new DFS<Ass>(a);
    delete a;
    nxt = cur = e->next();
    if (cur != NULL) nxt = e->next();
  }
  int
  CumulativeAssignment::operator[](int i) const {
    assert((i>=0) && (i<n) && cur != NULL);
    return cur->x[i].val();
  }
  void
  CumulativeAssignment::operator++(void) {
    if(cur) delete cur;
    cur = nxt;
    if (nxt != NULL) nxt = e->next();
  }
  bool
  CumulativeAssignment::operator()(void) const {
    return nxt != NULL;
  }

  // Classes and functions for checking an assignment
  struct Event {
    int p, h;
    bool start;
    Event(int pos, int height, bool s) : p(pos), h(height), start(s) {}
    bool operator<(const Event& e) const { return p<e.p; }
  };

  struct Below {
    int limit;
    Below(int l) : limit(l) {}
    bool operator()(int val) { return val <= limit; }
  };
  struct Above {
    int limit;
    Above(int l) : limit(l) {}
    bool operator()(int val) { return val >= limit; }
  };

  typedef std::vector<Event> eventv;

  template <class C>
  bool valid(eventv e, C comp) {
    std::sort(e.begin(), e.end());
    unsigned int i = 0;
    int p = 0;
    int h = 0;
    int n = 0;
    while (i < e.size()) {
      p = e[i].p;
      while (i < e.size() && e[i].p == p) {
        h += e[i].h;
        n += (e[i].start ? +1 : -1);
        ++i;
      }
      if (n && !comp(h)) {
        return false;
      }
    }
    return true;
  }
}

class Cumulatives : public IntTest {
protected:
  int ntasks;
  bool at_most;
  int limit;

public:
  /// Create and register test
  Cumulatives(const char* t, int nt, bool am, int l)
    : IntTest(t,nt*4,ds_12), ntasks(nt), at_most(am), limit(l) {}

  virtual Assignment* assignment(void) const {
    assert(arity == 4*ntasks);
    return new CumulativeAssignment(arity, dom);
  }
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    eventv e;
    for (int i = 0; i < ntasks; ++i) {
      int p = i*4;
      // Positive start, duration and end
      if (x[p+0] < 0 || x[p+1] < 1 || x[p+2] < 1) return false;
      // Start + Duration == End
      if (x[p+0] + x[p+1] != x[p+2]) {
        return false;
      }
    }
    for (int i = 0; i < ntasks; ++i) {
      int p = i*4;
      // Up at start, down at end.
      e.push_back(Event(x[p+0], +x[p+3],  true));
      e.push_back(Event(x[p+2], -x[p+3], false));
    }
    if (at_most) {
      return valid(e, Below(limit));
    } else {
      return valid(e, Above(limit));
    }
  }

  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntArgs m(ntasks), l(1, limit);
    IntVarArgs s(ntasks), d(ntasks), e(ntasks), h(ntasks);
    for (int i = 0; i < ntasks; ++i) {
      int p = i*4;
      m[i] = 0;
      s[i] = x[p+0]; rel(home, x[p+0], Gecode::IRT_GQ, 0);
      d[i] = x[p+1]; rel(home, x[p+1], Gecode::IRT_GQ, 1);
      e[i] = x[p+2]; rel(home, x[p+2], Gecode::IRT_GQ, 1);
      h[i] = x[p+3];
    }
    cumulatives(home, m, s, d, e, h, l, at_most);
  }
};

namespace {
  Cumulatives _cumu1t1("Cumulatives::1t1", 1,  true, 1);
  Cumulatives _cumu1f1("Cumulatives::1f1", 1, false, 1);
  Cumulatives _cumu1t2("Cumulatives::1t2", 1,  true, 2);
  Cumulatives _cumu1f2("Cumulatives::1f2", 1, false, 2);
  Cumulatives _cumu1t3("Cumulatives::1t3", 1,  true, 3);
  Cumulatives _cumu1f3("Cumulatives::1f3", 1, false, 3);
  Cumulatives _cumu2t1("Cumulatives::2t1", 2,  true, 1);
  Cumulatives _cumu2f1("Cumulatives::2f1", 2, false, 1);
  Cumulatives _cumu2t2("Cumulatives::2t2", 2,  true, 2);
  Cumulatives _cumu2f2("Cumulatives::2f2", 2, false, 2);
  Cumulatives _cumu2t3("Cumulatives::2t3", 2,  true, 3);
  Cumulatives _cumu2f3("Cumulatives::2f3", 2, false, 3);
  Cumulatives _cumu3t1("Cumulatives::3t1", 3,  true, 1);
  Cumulatives _cumu3f1("Cumulatives::3f1", 3, false, 1);
  Cumulatives _cumu3t2("Cumulatives::3t2", 3,  true, 2);
  Cumulatives _cumu3f2("Cumulatives::3f2", 3, false, 2);
  Cumulatives _cumu3t3("Cumulatives::3t3", 3,  true, 3);
  Cumulatives _cumu3f3("Cumulatives::3f3", 3, false, 3);
  /* These tests take quite some time to run.
  Cumulatives _cumu4t1("Cumulatives::4t1", 4,  true, 1);
  Cumulatives _cumu4f1("Cumulatives::4f1", 4, false, 1);
  Cumulatives _cumu4t2("Cumulatives::4t2", 4,  true, 2);
  Cumulatives _cumu4f2("Cumulatives::4f2", 4, false, 2);
  Cumulatives _cumu4t3("Cumulatives::4t3", 4,  true, 3);
  Cumulatives _cumu4f3("Cumulatives::4f3", 4, false, 3);
  */
}

}}}

// STATISTICS: test-int
