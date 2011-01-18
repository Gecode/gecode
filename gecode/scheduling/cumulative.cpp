/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *     Guido Tack, 2010
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
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::cumulative");
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    bool fixp = true;
    for (int i=t.size(); i--;)
      if (t[i] != TT_FIXP) {
        fixp = false; break;
      }
    if (fixp) {
      TaskArray<ManFixPTask> tasks(home,s.size());
      for (int i=0; i<s.size(); i++)
        tasks[i].init(s[i],p[i],u[i]);
      GECODE_ES_FAIL(ManProp<ManFixPTask>::post(home,c,tasks));
    } else {
      TaskArray<ManFixPSETask> tasks(home,s.size());
      for (int i=s.size(); i--;)
        tasks[i].init(t[i],s[i],p[i],u[i]);
      GECODE_ES_FAIL(ManProp<ManFixPSETask>::post(home,c,tasks));
    }
  }

  void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::cumulative");
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    bool fixp = true;
    for (int i=t.size(); i--;)
      if (t[i] != TT_FIXP) {
        fixp = false; break;
      }
    if (fixp) {
      TaskArray<OptFixPTask> tasks(home,s.size());
      for (int i=0; i<s.size(); i++)
        tasks[i].init(s[i],p[i],u[i],m[i]);
      GECODE_ES_FAIL(OptProp<OptFixPTask>::post(home,c,tasks));
    } else {
      TaskArray<OptFixPSETask> tasks(home,s.size());
      for (int i=s.size(); i--;)
        tasks[i].init(t[i],s[i],p[i],u[i],m[i]);
      GECODE_ES_FAIL(OptProp<OptFixPSETask>::post(home,c,tasks));
    }
  }
  void
  cumulative(Home home, int c, const IntVarArgs& s, 
             const IntArgs& p, const IntArgs& u) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::cumulative");
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    TaskArray<ManFixPTask> t(home,s.size());
    for (int i=0; i<s.size(); i++) {
      t[i].init(s[i],p[i],u[i]);
    }
    GECODE_ES_FAIL(ManProp<ManFixPTask>::post(home,c,t));
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::cumulative");
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    TaskArray<OptFixPTask> t(home,s.size());
    for (int i=0; i<s.size(); i++) {
      t[i].init(s[i],p[i],u[i],m[i]);
    }
    GECODE_ES_FAIL(OptProp<OptFixPTask>::post(home,c,t));
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, 
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != e.size()) ||
        (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i].max(),
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i].max()) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    TaskArray<ManFlexTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],e[i],u[i]);
    GECODE_ES_FAIL(ManProp<ManFlexTask>::post(home,c,t));
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != e.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::cumulative");
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(u[i],"Scheduling::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i].max(),
                         "Scheduling::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i].max()) * u[i],
                                "Scheduling::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c * w * s.size(),
                              "Scheduling::cumulative");
    if (home.failed()) return;
    TaskArray<OptFlexTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],e[i],u[i],m[i]);
    GECODE_ES_FAIL(OptProp<OptFlexTask>::post(home,c,t));
  }
  
}

// STATISTICS: scheduling-post
