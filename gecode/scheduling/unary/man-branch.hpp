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

namespace Gecode { namespace Scheduling { namespace Unary {

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
  ManBranch<Task>::ManBranch(Space& home, TaskArray<TaskBranch<Task> >& t0) 
    : Branching(home), t(t0), 
      n_non_first(t.size()), n_non_notfirst(t.size()) {
  }

  template<class Task>
  forceinline
  ManBranch<Task>::ManBranch(Space& home, bool share, ManBranch& b) 
    : Branching(home, share, b), 
      n_non_first(b.n_non_first), n_non_notfirst(b.n_non_notfirst) {
    t.update(home, share, b.t);
  }

  template<class Task>
  bool 
  ManBranch<Task>::status(const Space&) const {
    return (n_non_first >= 2) && (n_non_notfirst >= 2);
  }

  template<class Task>
  BranchingDesc* 
  ManBranch<Task>::description(Space&) {
    int j = -1;
    int est = Int::Limits::infinity;
    for (int i=0; i<t.size(); i++)
      if (t[i].available() && (t[i].est() < est)) {
        est=t[i].est(); j=i;
      }
    return new Description(*this,0,j);
  }

  template<class Task>
  ExecStatus 
  ManBranch<Task>::commit(Space& home, const BranchingDesc& _d, 
                          unsigned int a) {
    const Description& d = static_cast<const Description&>(_d);
    if (a == 0) {
      // Simple case: make t[d.t] go first, reset all notfirst
      t[d.t].first(true); n_non_first--;
      for (int i=t.size(); i--; ) {
        if (!t[i].first())
          GECODE_ES_CHECK(t[d.t].before(home, t[i]));
        t[i].notfirst(false);
      }
      n_non_notfirst = n_non_first;
    } else if (n_non_first == 2) {
      /*
       * Special case: there are only two tasks left that still.
       * can go first. We have to find the other task (different
       * from t[d.t]) and that must go before. After that it does 
       * not matter what we do as this is the last choice to be
       * made for the resource.
       */
      t[d.t].first(true); n_non_first--;
      for (int i=t.size(); i--; )
        if (t[i].available()) {
          GECODE_ES_CHECK(t[i].before(home, t[d.t]));
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
      t[d.t].notfirst(true); // So that d.t is skipped
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
       * The task d.t cannot be first. However we do not create a
       * propagator to enforce that but just make t.d start after
       * the task that can finish earliest.
       */
      t[d.t].notfirst(true); n_non_notfirst--;
      int ect = Int::Limits::infinity;
      for (int i=t.size(); i--; )
        if (t[i].available())
          ect = std::min(ect,t[i].ect());
      GECODE_ME_CHECK(t[d.t].est(home,ect));
    }
    return ES_OK;
  }

  template<class Task>
  Actor* 
  ManBranch<Task>::copy(Space& home, bool share) {
    return new (home) ManBranch<Task>(home, share, *this);
  }

  template<class Task>
  forceinline void 
  ManBranch<Task>::post(Space& home, TaskArray<TaskBranch<Task> >& t) {
    (void) new (home) ManBranch<Task>(home,t);
  }

  template<class Task>
  size_t
  ManBranch<Task>::dispose(Space& home) {
    return sizeof(*this);
  }

}}}

// STATISTICS: scheduling-branch
