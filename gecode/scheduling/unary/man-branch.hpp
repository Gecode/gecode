/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <climits>

namespace Gecode { namespace Scheduling { namespace Unary {

  template<class Task>
  forceinline
  ManResource<Task>::ManResource(void) {}

  template<class Task>
  forceinline void
  ManResource<Task>::init(TaskArray<TaskBranch<Task> >& t0) {
    t=t0; n_non_first=t.size(); n_non_notfirst=t.size();
  }

  template<class Task>
  forceinline bool 
  ManResource<Task>::status(void) const {
    return (n_non_first >= 2) && (n_non_notfirst >= 2);
  }

  template<class Task>
  forceinline int
  ManResource<Task>::task(void) const {
    int j = -1;
    int est = Int::Limits::infinity;
    int lct = Int::Limits::infinity;
    for (int i=0; i<t.size(); i++)
      if (t[i].available() && 
          ((t[i].est() < est) ||
           ((t[i].est() == est) && (t[i].lct() < lct)))) {
        est=t[i].est(); lct=t[i].lct(); j=i;
      }
    return j;
  }

  template<class Task>
  forceinline unsigned int
  ManResource<Task>::slack(void) const {
    int min_est = t[t.size()-1].est();
    int max_lst = t[t.size()-1].lst();
    unsigned int p = static_cast<unsigned int>(t[t.size()-1].p());
    for (int i=t.size()-1; i--; ) {
      min_est = std::min(min_est,t[i].est());
      max_lst = std::max(max_lst,t[i].lst());
      p += static_cast<unsigned int>(t[i].p());
    }
    return static_cast<unsigned int>(max_lst-min_est)-p;
  }

  template<class Task>
  forceinline void
  ManResource<Task>::update(Space& home, bool share, ManResource<Task>& m) {
    t.update(home,share,m.t);
    n_non_first=m.n_non_first; 
    n_non_notfirst=m.n_non_notfirst;
  }

  
  template<class Task>
  ExecStatus 
  ManResource<Task>::commit(Space& home, int d_t, unsigned int a) {
    if (a == 0) {
      // Simple case: make t[d_t] go first, reset all notfirst
      t[d_t].first(true); n_non_first--;
      for (int i=t.size(); i--; ) {
        if (!t[i].first())
          GECODE_ES_CHECK(t[d_t].before(home, t[i]));
        t[i].notfirst(false);
      }
      n_non_notfirst = n_non_first;
    } else if (n_non_first == 2) {
      /*
       * Special case: there are only two tasks left that still.
       * can go first. We have to find the other task (different
       * from t[d_t]) and that must go before. After that it does 
       * not matter what we do as this is the last choice to be
       * made for the resource.
       */
      t[d_t].first(true); n_non_first--;
      for (int i=t.size(); i--; )
        if (t[i].available()) {
          GECODE_ES_CHECK(t[i].before(home, t[d_t]));
          return ES_OK;
        }
      GECODE_NEVER;
    } else if (n_non_notfirst == 2) {
      /*
       * Special case: we have to propagate that all but one task
       * cannot be first. That means we have to find that task
       * and make it go first. We have to be careful to properly
       * erase the notfirst information as branching may continue.
       */
      t[d_t].notfirst(true); // So that d_t is skipped
      int f = -1;
      for (int i=t.size(); i--; ) {
        if (t[i].available()) {
          assert(f == -1);
          f = i;
        }
        t[i].notfirst(false);
      }
      t[f].first(true); n_non_first--;
      n_non_notfirst = n_non_first;
      for (int i=t.size(); i--; ) 
        if (!t[i].first())
          GECODE_ES_CHECK(t[f].before(home, t[i]));
    } else {
      /*
       * The task d_t cannot be first. However we do not create a
       * propagator to enforce that but just make t.d start after
       * the task that can finish earliest.
       */
      t[d_t].notfirst(true); n_non_notfirst--;
      int ect = Int::Limits::infinity;
      for (int i=t.size(); i--; )
        if (t[i].available())
          ect = std::min(ect,t[i].ect());
      GECODE_ME_CHECK(t[d_t].est(home,ect));
    }
    return ES_OK;
  }




  template<class Task>
  forceinline
  ManBranch<Task>::Description::Description(const Branching& b, int r0, int t0)
    : BranchingDesc(b,2), r(r0), t(t0) {}

  template<class Task>
  size_t
  ManBranch<Task>::Description::size(void) const {
    return sizeof(Description);
  }



  template<class Task>
  forceinline
  ManBranch<Task>::ManBranch(Space& home, ManResource<Task>* r0, int n0) 
    : Branching(home), n(n0), c(-1), r(r0) {}

  template<class Task>
  forceinline
  ManBranch<Task>::ManBranch(Space& home, bool share, ManBranch& b) 
    : Branching(home, share, b), n(b.n), c(b.c),
      r(home.alloc<ManResource<Task> >(n)) {
    for (int i=n; i--; )
      r[i].update(home, share, b.r[i]);
  }

  template<class Task>
  bool 
  ManBranch<Task>::status(const Space&) const {
    if ((c < 0) || !r[c].status()) {
      // Find resource with least slack
      c=-1;
      unsigned int s = UINT_MAX;
      for (int i=0; i<n; i++)
        if (r[i].status() && (r[i].slack() < s))
          c=i;
    }
    return (c >= 0);
  }

  template<class Task>
  BranchingDesc* 
  ManBranch<Task>::description(Space&) {
    return new Description(*this,c,r[c].task());
  }

  template<class Task>
  ExecStatus 
  ManBranch<Task>::commit(Space& home, const BranchingDesc& _d, 
                          unsigned int a) {
    const Description& d = static_cast<const Description&>(_d);
    return r[d.r].commit(home,d.t,a);
  }

  template<class Task>
  Actor* 
  ManBranch<Task>::copy(Space& home, bool share) {
    return new (home) ManBranch<Task>(home, share, *this);
  }

  template<class Task>
  forceinline void 
  ManBranch<Task>::post(Space& home, ManResource<Task>* r, int n) {
    (void) new (home) ManBranch<Task>(home,r,n);
  }

  template<class Task>
  size_t
  ManBranch<Task>::dispose(Space&) {
    return sizeof(*this);
  }

}}}

// STATISTICS: scheduling-branch
