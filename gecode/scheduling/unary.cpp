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

#include <algorithm>

namespace Gecode {

  void
  unary(Space& home, const IntVarArgs& s, const IntArgs& p) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::unary");
    if (s.size() != p.size())
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      if (p[i] <= 0)
        throw Int::OutOfLimits("Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    TaskArray<ManFixTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i]);
    GECODE_ES_FAIL(home,ManProp<ManFixTask>::post(home,t));
  }

  void
  unary(Space& home, const IntVarArgs& s, const IntArgs& p, 
        const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::unary");
    if ((s.size() != p.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      if (p[i] <= 0)
        throw Int::OutOfLimits("Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    TaskArray<OptFixTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],m[i]);
    GECODE_ES_FAIL(home,OptProp<OptFixTask>::post(home,t));
  }

  void 
  order(Space& home, const IntArgs& r, const IntVarArgs& s, const IntArgs& p) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::order");
    int n=s.size();
    if ((n != p.size()) || (n != r.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::order");
    for (int i=p.size(); i--; ) {
      if (p[i] <= 0)
        throw Int::OutOfLimits("Scheduling::order");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::order");
    }
    if (n < 2)
      return;
    if (home.failed()) return;
    // Find largest resource number
    int n_r = r[n-1];
    for (int i=n-1; i--; )
      n_r = std::max(n_r, r[i]);
    n_r++;
    Region re(home);
    // The number of tasks for a given resource
    int* n_t = re.alloc<int>(n_r);
    for (int i=n_r; i--; )
      n_t[i]=0;
    for (int i=n; i--; )
      n_t[r[i]]++;
    // resource information data structure
    ManResource<ManFixTask>* ri = 
      home.alloc<ManResource<ManFixTask> >(n_r);
    // Initialize information
    for (int i=n_r; i--; ) {
      TaskArray<TaskBranch<ManFixTask> > t(home,n_t[i]);
      int j=0;
      for (int k=n; k--; )
        if (r[k] == i)
          t[j++].init(s[k],p[k]);
      assert(j == n_t[i]);
      ri[i].init(t);
    }
    ManBranch<ManFixTask>::post(home,ri,n_r);
  }

}

// STATISTICS: scheduling-post
