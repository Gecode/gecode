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

#include <gecode/scheduling/unary.hh>

#include <algorithm>

namespace Gecode {

  void
  unary(Home home, const IntVarArgs& s, const IntArgs& p) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::unary");
    if (s.size() != p.size())
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    TaskArray<ManFixPTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i]);
    GECODE_ES_FAIL(ManProp<ManFixPTask>::post(home,t));
  }

  void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& s, const IntArgs& p) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if ((s.size() != p.size()) || (s.size() != t.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      if (t[i] == TT_FIXP)
        Int::Limits::nonnegative(p[i],"Scheduling::unary");
      else
        Int::Limits::check(p[i],"Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    bool fixp = true;
    for (int i=t.size(); i--;)
      if (t[i] != TT_FIXP) {
        fixp = false; break;
      }
    if (fixp) {
      TaskArray<ManFixPTask> tasks(home,s.size());
      for (int i=s.size(); i--; )
        tasks[i].init(s[i],p[i]);
      GECODE_ES_FAIL(ManProp<ManFixPTask>::post(home,tasks));
    } else {
      TaskArray<ManFixPSETask> tasks(home,s.size());
      for (int i=s.size(); i--;)
        tasks[i].init(t[i],s[i],p[i]);
      GECODE_ES_FAIL(ManProp<ManFixPSETask>::post(home,tasks));
    }
  }

  void
  unary(Home home, const IntVarArgs& s, const IntArgs& p, 
        const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if (s.same(home))
      throw Int::ArgumentSame("Scheduling::unary");
    if ((s.size() != p.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    TaskArray<OptFixPTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],m[i]);
    GECODE_ES_FAIL(OptProp<OptFixPTask>::post(home,t));
  }

  void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& s, const IntArgs& p, const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if ((s.size() != p.size()) || (s.size() != t.size()) ||
        (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    for (int i=p.size(); i--; ) {
      if (t[i] == TT_FIXP)
        Int::Limits::nonnegative(p[i],"Scheduling::unary");
      else
        Int::Limits::check(p[i],"Scheduling::unary");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Scheduling::unary");
    }
    if (home.failed()) return;
    bool fixp = true;
    for (int i=t.size(); i--;)
      if (t[i] != TT_FIXP) {
        fixp = false; break;
      }
    if (fixp) {
      TaskArray<OptFixPTask> tasks(home,s.size());
      for (int i=s.size(); i--; )
        tasks[i].init(s[i],p[i],m[i]);
      GECODE_ES_FAIL(OptProp<OptFixPTask>::post(home,tasks));
    } else {
      TaskArray<OptFixPSETask> tasks(home,s.size());
      for (int i=s.size(); i--;)
        tasks[i].init(t[i],s[i],p[i],m[i]);
      GECODE_ES_FAIL(OptProp<OptFixPSETask>::post(home,tasks));
    }
  }

  void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if ((s.size() != p.size()) || (s.size() != e.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    if (home.failed()) return;
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    TaskArray<ManFlexTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],e[i]);
    GECODE_ES_FAIL(ManProp<ManFlexTask>::post(home,t));
  }

  void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p, 
        const IntVarArgs& e, const BoolVarArgs& m) {
    using namespace Gecode::Scheduling;
    using namespace Gecode::Scheduling::Unary;
    if ((s.size() != p.size()) || (s.size() != m.size()) ||
        (s.size() != e.size()))
      throw Int::ArgumentSizeMismatch("Scheduling::unary");
    if (home.failed()) return;
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    TaskArray<OptFlexTask> t(home,s.size());
    for (int i=s.size(); i--; )
      t[i].init(s[i],p[i],e[i],m[i]);
    GECODE_ES_FAIL(OptProp<OptFlexTask>::post(home,t));
  }

}

// STATISTICS: scheduling-post
