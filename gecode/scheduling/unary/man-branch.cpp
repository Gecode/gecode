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
      /// Positions of tasks to be ordered
      int* t;
      /** Initialize description for branching \a b, number of
       *  alternatives \a a, resource \a r0, and tasks \a t0.
       */
      Description(const Branching& b, unsigned int a, int r0, int* t0)
        : BranchingDesc(b,a), r(r0), t(t0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Description);
      }
      /// Destructor
      virtual ~Description(void) {
        heap.free<int>(t,alternatives());
      }
    };
    /// The tasks
    TaskArray<Task> t;
    /// Whether a task has already been ordered
    bool* ordered;
    /// The number of not yet ordered tasks
    int n_unordered;
    /// Construct branching
    ManBranching(Space& home, TaskArray<Task>& t0) 
      : Branching(home), 
        t(t0), ordered(home.alloc<bool>(t.size())), n_unordered(t.size()) {
      for (int i=t.size(); i--; )
        ordered[i] = false;
    }
    /// Copy constructor
    ManBranching(Space& home, bool share, ManBranching& b) 
      : Branching(home, share, b), 
        ordered(home.alloc<bool>(b.t.size())), n_unordered(b.n_unordered) {
      t.update(home, share, b.t);
      for (int i=t.size(); i--; )
        ordered[i] = b.ordered[i];
    }
  public:
    /// Check status of branching, return true if alternatives left
    virtual bool status(const Space&) const {
      return n_unordered >= 2;
    }
    /// Return branching description
    virtual BranchingDesc* description(Space&) {
      int* ti = heap.alloc<int>(n_unordered);
      int a = 0;
      for (int i=0; i<t.size(); i++)
        if (!ordered[i])
          ti[a++]=i;
      assert(a == n_unordered);
      return new Description(*this,static_cast<unsigned int>(n_unordered),
                             0,ti);
    }
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space& home, const BranchingDesc& _d, 
                              unsigned int a) {
      const Description& d = static_cast<const Description&>(_d);
      for (unsigned int i=0; i<d.alternatives(); i++) 
        if (a != i)
          GECODE_ES_CHECK(manbefore(home,t[d.t[a]],t[d.t[i]]));
      ordered[d.t[a]] = true;
      n_unordered--;
      return ES_OK;
    }
    /// Copy branching
    virtual Actor* copy(Space& home, bool share) {
      return new (home) ManBranching<Task>(home, share, *this);
    }
    /// Post branching
    static void post(Space& home, TaskArray<Task>& t) {
      std::cout << "post(" << t << ")" << std::endl;
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
