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

#include <gecode/scheduling/cumulative.hh>

#include <algorithm>

namespace Gecode {

  void
  cumulative(Home home, int c, const IntVarArgs& s, 
             const IntArgs& p, const IntArgs& u) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::cumulative");
    if ((s.size() != p.size()) || (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    for (int i=p.size(); i--; ) {
      Int::Limits::positive(p[i],"Scheduling::cumulative");
      Int::Limits::positive(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(p[i]) * u[i],
                         "Scheduling::cumulative");
    }
    if (home.failed()) return;
    TaskArray<ManFixTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],u[i]);
    GECODE_ES_FAIL(home,ManProp<ManFixTask>::post(home,c,t));
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::cumulative");
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    for (int i=p.size(); i--; ) {
      Int::Limits::positive(p[i],"Scheduling::cumulative");
      Int::Limits::positive(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(p[i]) * u[i],
                         "Scheduling::cumulative");
    }
    if (home.failed()) return;
    TaskArray<OptFixTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],u[i],m[i]);
    GECODE_ES_FAIL(home,OptProp<OptFixTask>::post(home,c,t));
  }

}

// STATISTICS: scheduling-post
