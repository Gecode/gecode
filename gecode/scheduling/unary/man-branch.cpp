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

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Branching for mandatory tasks
  template<class Task>
  class ManBranching : public Branching {
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
    TaskArray<Task> t;
    /// Whether a task has already been ordered
    bool* ordered;
    /// Whether a task is blocked until another task is ordered
    bool* blocked;
    /// The number of tasks that are available for branching
    int n_available;
    /// Construct branching
    ManBranching(Space& home, TaskArray<Task>& t0) 
      : Branching(home), t(t0), 
        ordered(home.alloc<bool>(t.size())), 
        blocked(home.alloc<bool>(t.size())), 
        n_available(t.size()) {
      for (int i=t.size(); i--; )
        ordered[i] = blocked[i] = false;
    }
    /// Copy constructor
    ManBranching(Space& home, bool share, ManBranching& b) 
      : Branching(home, share, b), 
        ordered(home.alloc<bool>(b.t.size())), 
        blocked(home.alloc<bool>(b.t.size())), 
        n_available(b.n_available) {
      t.update(home, share, b.t);
      for (int i=t.size(); i--; ) {
        ordered[i] = b.ordered[i];
        blocked[i] = b.blocked[i];
      }
    }
  public:
    /// Check status of branching, return true if alternatives left
    virtual bool status(const Space&) const {
      return n_available >= 2;
    }
    /// Return branching description
    virtual BranchingDesc* description(Space&) {
      int j = -1;
      int est = Int::Limits::infinity;
      for (int i=0; i<t.size(); i++)
        if (!ordered[i] && !blocked[i] && (t[i].est() < est)) {
          est=t[i].est(); j=i;
        }
      return new Description(*this,0,j);
    }
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space& home, const BranchingDesc& _d, 
                              unsigned int a) {
      const Description& d = static_cast<const Description&>(_d);
      if (a == 0) {
        for (int i=t.size(); i--; ) {
          if ((i != d.t) && !ordered[i])
            GECODE_ES_CHECK(manbefore(home,t[d.t],t[i]));
          if (blocked[i]) {
            blocked[i] = false;
            n_available++;
          }
        }
        ordered[d.t] = true;
        n_available--;
      } else {
        n_available--;
        blocked[d.t] = true;
        if (n_available == 1) {
          for (int i=t.size(); i--; )
            if (!ordered[i] && !blocked[i]) {
              GECODE_ES_CHECK(manbefore(home,t[i],t[d.t]));
              return ES_OK;
            }
        } 
        int ect = Int::Limits::infinity;
        for (int i=t.size(); i--; )
          if (!ordered[i] && !blocked[i])
            ect = std::min(ect,t[i].ect());
        GECODE_ME_CHECK(t[d.t].est(home,ect));
      }
      return ES_OK;
    }
    /// Copy branching
    virtual Actor* copy(Space& home, bool share) {
      return new (home) ManBranching<Task>(home, share, *this);
    }
    /// Post branching
    static void post(Space& home, TaskArray<Task>& t) {
      (void) new (home) ManBranching<Task>(home,t);
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
    TaskArray<ManFixTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i]);
    ManBranching<ManFixTask>::post(home,t);
  }

}

// STATISTICS: scheduling-branch
