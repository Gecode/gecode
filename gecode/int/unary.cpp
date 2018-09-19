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

#include <gecode/int/unary.hh>
#include <gecode/int/distinct.hh>

#include <algorithm>

namespace Gecode {

  void
  unary(Home home, const IntVarArgs& s, const IntArgs& p, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if (same(s))
      throw Int::ArgumentSame("Int::unary");
    if (s.size() != p.size())
      throw Int::ArgumentSizeMismatch("Int::unary");
    for (int i=0; i<p.size(); i++) {
      Int::Limits::nonnegative(p[i],"Int::unary");
      Int::Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                         "Int::unary");
    }
    GECODE_POST;
    bool allOne = true;
    for (int i=0; i<p.size(); i++) {
      if (p[i] != 1) {
        allOne = false;
        break;
      }
    }
    if (allOne) {
      ViewArray<IntView> xv(home,s);
      switch (vbd(ipl)) {
      case IPL_BND:
        GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home,xv));
        break;
      case IPL_DOM:
        GECODE_ES_FAIL(Distinct::Dom<IntView>::post(home,xv));
        break;
      default:
        GECODE_ES_FAIL(Distinct::Val<IntView>::post(home,xv));
      }
    } else {
      TaskArray<ManFixPTask> t(home,s.size());
      for (int i=0; i<s.size(); i++)
        t[i].init(s[i],p[i]);
      GECODE_ES_FAIL(manpost(home,t,ipl));
    }
  }

  void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& flex, const IntArgs& fix, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if ((flex.size() != fix.size()) || (flex.size() != t.size()))
      throw Int::ArgumentSizeMismatch("Int::unary");
    for (int i=0; i<fix.size(); i++) {
      if (t[i] == TT_FIXP)
        Int::Limits::nonnegative(fix[i],"Int::unary");
      else
        Int::Limits::check(fix[i],"Int::unary");
      Int::Limits::check(static_cast<long long int>(flex[i].max()) + fix[i],
                         "Int::unary");
    }
    GECODE_POST;
    bool fixp = true;
    for (int i=0; i<t.size(); i++)
      if (t[i] != TT_FIXP) {
        fixp = false; break;
      }
    if (fixp) {
      unary(home, flex, fix, ipl);
    } else {
      TaskArray<ManFixPSETask> tasks(home,flex.size());
      for (int i=0; i<flex.size(); i++)
        tasks[i].init(t[i],flex[i],fix[i]);
      GECODE_ES_FAIL(manpost(home,tasks,ipl));
    }
  }

  void
  unary(Home home, const IntVarArgs& s, const IntArgs& p,
        const BoolVarArgs& m, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if (same(s))
      throw Int::ArgumentSame("Int::unary");
    if ((s.size() != p.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::unary");
    for (int i=0; i<p.size(); i++) {
      Int::Limits::nonnegative(p[i],"Int::unary");
      Int::Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                         "Int::unary");
    }
    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      unary(home,s,p,ipl);
    } else {
      GECODE_POST;
      TaskArray<OptFixPTask> t(home,s.size());
      for (int i=0; i<s.size(); i++)
        t[i].init(s[i],p[i],m[i]);
      GECODE_ES_FAIL(optpost(home,t,ipl));
    }
  }

  void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& flex, const IntArgs& fix, const BoolVarArgs& m,
        IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if ((flex.size() != fix.size()) || (flex.size() != t.size()) ||
        (flex.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::unary");
    bool fixp = true;
    for (int i=0; i<fix.size(); i++) {
      if (t[i] == TT_FIXP) {
        Int::Limits::nonnegative(fix[i],"Int::unary");
      } else {
        fixp = false;
        Int::Limits::check(fix[i],"Int::unary");
      }
      Int::Limits::check(static_cast<long long int>(flex[i].max()) + fix[i],
                         "Int::unary");
    }
    GECODE_POST;
    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      unary(home,t,flex,fix,ipl);
    } else {
      if (fixp) {
        TaskArray<OptFixPTask> tasks(home,flex.size());
        for (int i=0; i<flex.size(); i++)
          tasks[i].init(flex[i],fix[i],m[i]);
        GECODE_ES_FAIL(optpost(home,tasks,ipl));
      } else {
        TaskArray<OptFixPSETask> tasks(home,flex.size());
        for (int i=0; i<flex.size(); i++)
          tasks[i].init(t[i],flex[i],fix[i],m[i]);
        GECODE_ES_FAIL(optpost(home,tasks,ipl));
      }
    }
  }

  void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if ((s.size() != p.size()) || (s.size() != e.size()))
      throw Int::ArgumentSizeMismatch("Int::unary");
    GECODE_POST;
    for (int i=0; i<p.size(); i++) {
      IntView pi(p[i]);
      GECODE_ME_FAIL(pi.gq(home,0));
    }
    bool fixP = true;
    for (int i=0; i<p.size(); i++) {
      if (!p[i].assigned()) {
        fixP = false;
        break;
      }
    }
    if (fixP) {
      IntArgs pp(p.size());
      for (int i=0; i<p.size(); i++)
        pp[i] = p[i].val();
      unary(home,s,pp,ipl);
    } else {
      TaskArray<ManFlexTask> t(home,s.size());
      for (int i=0; i<s.size(); i++)
        t[i].init(s[i],p[i],e[i]);
      GECODE_ES_FAIL(manpost(home,t,ipl));
    }
  }

  void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e, const BoolVarArgs& m, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Unary;
    if ((s.size() != p.size()) || (s.size() != m.size()) ||
        (s.size() != e.size()))
      throw Int::ArgumentSizeMismatch("Int::unary");
    GECODE_POST;
    for (int i=0; i<p.size(); i++) {
      IntView pi(p[i]);
      GECODE_ME_FAIL(pi.gq(home,0));
    }
    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      unary(home,s,p,e,ipl);
    } else {
      TaskArray<OptFlexTask> t(home,s.size());
      for (int i=0; i<s.size(); i++)
        t[i].init(s[i],p[i],e[i],m[i]);
      GECODE_ES_FAIL(optpost(home,t,ipl));
    }
  }

}

// STATISTICS: int-post
