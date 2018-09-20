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

#include <gecode/int/cumulative.hh>

#include <algorithm>

namespace Gecode { namespace Int { namespace Cumulative {

  template<class Cap>
  void
  cumulative(Home home, Cap c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             IntPropLevel ipl) {
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()))
      throw ArgumentSizeMismatch("Int::cumulative");
    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(p[i],"Int::cumulative");
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                    "Int::cumulative");
      mul_check(p[i],u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());
    GECODE_POST;

    int minU = INT_MAX; int minU2 = INT_MAX; int maxU = INT_MIN;
    for (int i=0; i<u.size(); i++) {
      if (u[i] < minU) {
        minU2 = minU;
        minU = u[i];
      } else if (u[i] < minU2)
        minU2 = u[i];
      if (u[i] > maxU)
        maxU = u[i];
    }
    bool disjunctive =
      (minU > c.max()/2) || (minU2 > c.max()/2 && minU+minU2>c.max());
    if (disjunctive) {
      GECODE_ME_FAIL(c.gq(home,maxU));
      unary(home,t,s,p,ipl);
    } else {
      bool fixp = true;
      for (int i=0; i<t.size(); i++)
        if (t[i] != TT_FIXP) {
          fixp = false; break;
        }
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      if (fixp) {
        TaskArray<ManFixPTask> tasks(home,nonOptionals);
        int cur = 0;
        for (int i=0; i<s.size(); i++)
          if (u[i] > 0)
            tasks[cur++].init(s[i],p[i],u[i]);
        GECODE_ES_FAIL(manpost(home,c,tasks,ipl));
      } else {
        TaskArray<ManFixPSETask> tasks(home,nonOptionals);
        int cur = 0;
        for (int i=0; i<s.size(); i++)
          if (u[i] > 0)
            tasks[cur++].init(t[i],s[i],p[i],u[i]);
        GECODE_ES_FAIL(manpost(home,c,tasks,ipl));
      }
    }
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(p[i],"Int::cumulative");
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                    "Int::cumulative");
      mul_check(p[i],u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());
    GECODE_POST;

    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,t,s,p,u,ipl);
    } else {
      bool fixp = true;
      for (int i=0; i<t.size(); i++)
        if (t[i] != TT_FIXP) {
          fixp = false; break;
        }
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      if (fixp) {
        TaskArray<OptFixPTask> tasks(home,nonOptionals);
        int cur = 0;
        for (int i=0; i<s.size(); i++)
          if (u[i]>0)
            tasks[cur++].init(s[i],p[i],u[i],m[i]);
        GECODE_ES_FAIL(optpost(home,c,tasks,ipl));
      } else {
        TaskArray<OptFixPSETask> tasks(home,nonOptionals);
        int cur = 0;
        for (int i=0; i<s.size(); i++)
          if (u[i]>0)
            tasks[cur++].init(t[i],s[i],p[i],u[i],m[i]);
        GECODE_ES_FAIL(optpost(home,c,tasks,ipl));
      }
    }
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s,
             const IntArgs& p, const IntArgs& u, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(p[i],"Int::cumulative");
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                    "Int::cumulative");
      mul_check(p[i],u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());
    GECODE_POST;

    int minU = INT_MAX; int minU2 = INT_MAX; int maxU = INT_MIN;
    for (int i=0; i<u.size(); i++) {
      if (u[i] < minU) {
        minU2 = minU;
        minU = u[i];
      } else if (u[i] < minU2)
        minU2 = u[i];
      if (u[i] > maxU)
        maxU = u[i];
    }
    bool disjunctive =
      (minU > c.max()/2) || (minU2 > c.max()/2 && minU+minU2>c.max());
    if (disjunctive) {
      GECODE_ME_FAIL(c.gq(home,maxU));
      unary(home,s,p,ipl);
    } else {
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      TaskArray<ManFixPTask> t(home,nonOptionals);
      int cur = 0;
      for (int i=0; i<s.size(); i++)
        if (u[i]>0)
          t[cur++].init(s[i],p[i],u[i]);
      GECODE_ES_FAIL(manpost(home,c,t,ipl));
    }
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, const BoolVarArgs& m, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(p[i],"Int::cumulative");
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i],
                    "Int::cumulative");
      mul_check(p[i],u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());
    GECODE_POST;

    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,s,p,u,ipl);
    } else {
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      TaskArray<OptFixPTask> t(home,nonOptionals);
      int cur = 0;
      for (int i=0; i<s.size(); i++)
        if (u[i]>0)
          t[cur++].init(s[i],p[i],u[i],m[i]);
      GECODE_ES_FAIL(optpost(home,c,t,ipl));
    }
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s,
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != e.size()) ||
        (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i].max(),
                    "Int::cumulative");
      mul_check(p[i].max(),u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());

    GECODE_POST;
    for (int i=0; i<p.size(); i++)
      GECODE_ME_FAIL(IntView(p[i]).gq(home,0));

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
      cumulative(home,c,s,pp,u,ipl);
    } else {
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      TaskArray<ManFlexTask> t(home,nonOptionals);
      int cur = 0;
      for (int i=0; i<s.size(); i++)
        if (u[i]>0)
          t[cur++].init(s[i],p[i],e[i],u[i]);
      GECODE_ES_FAIL(manpost(home,c,t,ipl));
    }
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m,
             IntPropLevel ipl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != e.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");

    long long int w = 0;
    for (int i=0; i<p.size(); i++) {
      Limits::nonnegative(u[i],"Int::cumulative");
      Limits::check(static_cast<long long int>(s[i].max()) + p[i].max(),
                    "Int::cumulative");
      mul_check(p[i].max(),u[i]);
      w += s[i].width();
    }
    mul_check(c.max(),w,s.size());

    GECODE_POST;
    for (int i=0; i<p.size(); i++)
      GECODE_ME_FAIL(IntView(p[i]).gq(home,0));

    bool allMandatory = true;
    for (int i=0; i<m.size(); i++) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,s,p,e,u,ipl);
    } else {
      int nonOptionals = 0;
      for (int i=0; i<u.size(); i++)
        if (u[i]>0) nonOptionals++;
      TaskArray<OptFlexTask> t(home,nonOptionals);
      int cur = 0;
      for (int i=0; i<s.size(); i++)
        if (u[i]>0)
          t[cur++].init(s[i],p[i],e[i],u[i],m[i]);
      GECODE_ES_FAIL(optpost(home,c,t,ipl));
    }
  }

}}}

namespace Gecode {

  void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),t,s,p,u,ipl);
  }

  void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),t,s,p,u,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),t,s,p,u,ipl);
  }


  void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),t,s,p,u,m,ipl);
  }

  void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),t,s,p,u,m,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),t,s,p,u,m,ipl);
  }


  void
  cumulative(Home home, int c, const IntVarArgs& s,
             const IntArgs& p, const IntArgs& u, IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),s,p,u,ipl);
  }

  void
  cumulative(Home home, IntVar c, const IntVarArgs& s,
             const IntArgs& p, const IntArgs& u, IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,u,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),s,p,u,ipl);
  }


  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, const BoolVarArgs& m, IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),s,p,u,m,ipl);
  }

  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, const BoolVarArgs& m, IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,u,m,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),s,p,u,m,ipl);
  }


  void
  cumulative(Home home, int c, const IntVarArgs& s,
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),s,p,e,u,ipl);
  }

  void
  cumulative(Home home, IntVar c, const IntVarArgs& s,
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,e,u,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),s,p,e,u,ipl);
  }


  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m,
             IntPropLevel ipl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    Int::Cumulative::cumulative(home,Int::ConstIntView(c),s,p,e,u,m,ipl);
  }

  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m,
             IntPropLevel ipl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,e,u,m,ipl);
    else
      Int::Cumulative::cumulative(home,Int::IntView(c),s,p,e,u,m,ipl);
  }

}

// STATISTICS: int-post
