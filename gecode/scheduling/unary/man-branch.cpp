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

#include <gecode/scheduling/unary.hh>
#include <gecode/int/linear.hh>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Add branching information to a task
  template<class Task>
  class Branch : public Task {
  protected:
    /// \name Branching information flags
    //@{
    static const char BIF_NONE = 0;     ///< Task can both
    static const char BIF_FIRST = 1;    ///< Task can go first if not set
    static const char BIF_NOTFIRST = 2; ///< Task can go not first if not set
    char bif;
    //@}
  public:
    /// Initialize that task can go first and not first
    Branch(void) : bif(BIF_NONE) {}
    /// Whether this task is first
    bool first(void) const {
      return (bif & BIF_FIRST) != BIF_NONE;
    }
    /// Set whether this task is first to \a b
    void first(bool b) {
      if (b) {
        bif |= BIF_FIRST;
      } else {
        bif &= ~BIF_FIRST;
      }
    }
    /// Whether this task is not first
    bool notfirst(void) const {
      return (bif & BIF_NOTFIRST) != BIF_NONE;
    }
    /// Set whether this task is not first to \a b
    void notfirst(bool b) {
      if (b) {
        bif |= BIF_NOTFIRST;
      } else {
        bif &= ~BIF_NOTFIRST;
      }
    }
    /// Whether this task is available
    bool available(void) const {
      return bif == BIF_NONE;
    }
    /// Also update information
    void update(Space& home, bool share, Branch<Task>& b) {
      Task::update(home,share,b);
      bif = b.bif;
    }
    /// Create propagator that this task is before task \a t
    forceinline ExecStatus
    before(Space& home, Branch<Task>& t) {
      Int::Linear::Term<Int::IntView> ax[2];
      ax[0].a=1; ax[0].x=st(); ax[1].a=-1; ax[1].x=t.st();
      Int::Linear::post(home, ax, 2, IRT_LQ, -p());
      return home.failed() ? ES_FAILED : ES_OK;
    }
  };


  /// Branching for mandatory tasks
  template<class Task>
  class ManBranch : public Branching {
  protected:
    /// Branching description
    class Description : public BranchingDesc {
    public:
      /// Number of resource
      int r;
      /// Positions of task to be ordered
      int t;
      /** Initialize description for branching \a b, 
       *  resource \a r0, and task \a t0.
       */
      Description(const Branching& b, int r0, int t0)
        : BranchingDesc(b,2), r(r0), t(t0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Description);
      }
    };
    /// The tasks
    TaskArray<Branch<Task> > t;
    /// The number of tasks that are non first
    int n_non_first;
    /// The number of tasks that are non not first
    int n_non_notfirst;
    /// Construct branching
    ManBranch(Space& home, TaskArray<Branch<Task> >& t0) 
      : Branching(home), t(t0), 
        n_non_first(t.size()), n_non_notfirst(t.size()) {
    }
    /// Copy constructor
    ManBranch(Space& home, bool share, ManBranch& b) 
      : Branching(home, share, b), 
        n_non_first(b.n_non_first), n_non_notfirst(b.n_non_notfirst) {
      t.update(home, share, b.t);
    }
  public:
    /// Check status of branching, return true if alternatives left
    virtual bool status(const Space&) const {
      return (n_non_first >= 2) && (n_non_notfirst >= 2);
    }
    /// Return branching description
    virtual BranchingDesc* description(Space&) {
      int j = -1;
      int est = Int::Limits::infinity;
      for (int i=0; i<t.size(); i++)
        if (t[i].available() && (t[i].est() < est)) {
          est=t[i].est(); j=i;
        }
      return new Description(*this,0,j);
    }
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space& home, const BranchingDesc& _d, 
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
    /// Copy branching
    virtual Actor* copy(Space& home, bool share) {
      return new (home) ManBranch<Task>(home, share, *this);
    }
    /// Post branching
    static void post(Space& home, TaskArray<Branch<Task> >& t) {
      (void) new (home) ManBranch<Task>(home,t);
    }
    /// Delete branching and return its size
    virtual size_t dispose(Space& home) {
      return sizeof(*this);
    }
  };

}}

  void 
  manbranch(Space& home, const IntVarArgs& s, const IntArgs& p) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::manbranch");
    if (s.size() != p.size())
      throw Int::ArgumentSizeMismatch("Scheduling::manbranch");
    for (int i=p.size(); i--; )
      if (p[i] <= 0)
        throw Int::OutOfLimits("Scheduling::manbranch");
    if (home.failed()) return;
    TaskArray<Branch<ManFixTask> > t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i]);
    ManBranch<ManFixTask>::post(home,t);
  }

}

// STATISTICS: scheduling-branch
