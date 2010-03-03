/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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

#include <gecode/scheduling/cumulatives.hh>
#include <gecode/int/linear.hh>

namespace Gecode {

  using namespace Int;

  namespace {
    ViewArray<IntView>
    make_view_array(Space& home, const IntVarArgs& in) {
      return ViewArray<Int::IntView>(home, in);
    }

    ViewArray<ConstIntView>
    make_view_array(Space& home, const IntArgs& in) {
      ViewArray<Int::ConstIntView> res(home, in.size());
      for (int i = in.size(); i--; ) {
        Int::Limits::check(in[i],"Scheduling::cumulatives");
        res[i] = Int::ConstIntView(in[i]);
      }

      return res;
    }

    template<class In> class ViewType;

    template<>
    class ViewType<IntArgs> {
    public:
      typedef Int::ConstIntView Result;
    };

    template<>
    class ViewType<IntVarArgs> {
    public:
      typedef Int::IntView Result;
    };

    void
    sum(Home home, IntVar s, IntVar d, IntVar e) {
      Int::Linear::Term<Int::IntView> t[3];
      t[0].a= 1; t[0].x=s;
      t[1].a= 1; t[1].x=d;
      t[2].a=-1; t[2].x=e;
      Int::Linear::post(home, t, 3, IRT_EQ, 0);
    }

    void
    sum(Home home, IntVar s, int d, IntVar e) {
      Int::Linear::Term<Int::IntView> t[2];
      t[0].a= 1; t[0].x=s;
      t[1].a=-1; t[1].x=e;
      Int::Linear::post(home, t, 2, IRT_EQ, -d);
    }

    template<class Machine, class Duration, class Height>
    void
    post_cumulatives(Home home, const Machine& machine,
                     const IntVarArgs& start, const Duration& duration,
                     const IntVarArgs& end, const Height& height,
                     const IntArgs& limit, bool at_most,
                     IntConLevel) {
      if (machine.size() != start.size()  ||
          start.size() != duration.size() ||
          duration.size() != end.size()   ||
          end.size() != height.size())
        throw Int::ArgumentSizeMismatch("Scheduling::cumulatives");
      if (home.failed()) return;

      ViewArray<typename ViewType<Machine>::Result>
        vmachine  = make_view_array(home,  machine);
      ViewArray<typename ViewType<Duration>::Result>
        vduration = make_view_array(home, duration);
      ViewArray<typename ViewType<Height>::Result>
        vheight   = make_view_array(home,   height);
      ViewArray<IntView>
        vstart    = make_view_array(home,    start),
        vend      = make_view_array(home,      end);

      SharedArray<int> limit_s(limit.size());
      for (int i=limit.size(); i--;)
        limit_s[i] = limit[i];

      // There is only the value-consistent propagator for this constraint
      GECODE_ES_FAIL((Scheduling::Cumulatives::Val<
                           typename ViewType<Machine>::Result,
                           typename ViewType<Duration>::Result,
                           typename ViewType<Height>::Result,
                           IntView>::post(home, vmachine,vstart, vduration,
                                          vend, vheight,limit_s, at_most)));

      // By definition, s+d=e should hold.
      // We enforce this using basic linear constraints, since the
      // sweep-algorithm does not check for it.
      for (int i = start.size(); i--; )
        sum(home, start[i], duration[i], end[i]);
    }
  }


  void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

}

// STATISTICS: scheduling-post
