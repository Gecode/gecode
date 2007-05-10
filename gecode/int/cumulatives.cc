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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int/cumulatives.hh"

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  namespace {
    ViewArray<IntView>
    make_view_array(Space *home, const IntVarArgs& in) {
      return ViewArray<IntView>(home, in);
    }

    ViewArray<ConstIntView>
    make_view_array(Space *home, const IntArgs& in) {
      ViewArray<ConstIntView> res(home, in.size());
      for (int i = in.size(); i--; ) {
        if (in[i] < Limits::Int::int_min ||
            in[i] > Limits::Int::int_max)
          throw new NumericalOverflow("Int::cumulatives");
        res[i] = ConstIntView(in[i]);
      }

      return res;
    }

    template<class In> class ViewType;

    template<>
    class ViewType<IntArgs> {
    public:
      typedef ConstIntView Result;
    };

    template<>
    class ViewType<IntVarArgs> {
    public:
      typedef IntView Result;
    };

    void
    sum(Space *home, const IntVar& s, const IntVar& d, const IntVar& e) {
      GECODE_AUTOARRAY(Linear::Term<IntView>, t, 3);
      t[0].a= 1; t[0].x=s;
      t[1].a= 1; t[1].x=d;
      t[2].a=-1; t[2].x=e;
      Linear::post(home, t, 3, IRT_EQ, 0);
    }

    void
    sum(Space *home, const IntVar& s, int d, const IntVar& e) {
      GECODE_AUTOARRAY(Linear::Term<IntView>, t, 2);
      t[0].a= 1; t[0].x=s;
      t[1].a=-1; t[1].x=e;
      Linear::post(home, t, 2, IRT_EQ, -d);
    }

    template <class Machine, class Duration, class Height>
    void
    post_cumulatives(Space* home, const Machine& machine,
                     const IntVarArgs& start, const Duration& duration,
                     const IntVarArgs& end, const Height& height,
                     const IntArgs& limit, bool at_most,
                     IntConLevel cl) {
      if (home->failed()) return;

      if (machine.size() != start.size()  ||
          start.size() != duration.size() ||
          duration.size() != end.size()   ||
          end.size() != height.size())
        throw new ArgumentSizeMismatch("Int::cumulatives");

      ViewArray<typename ViewType<Machine>::Result>
        vmachine  = make_view_array(home,  machine);
      ViewArray<typename ViewType<Duration>::Result>
        vduration = make_view_array(home, duration);
      ViewArray<typename ViewType<Height>::Result>
        vheight   = make_view_array(home,   height);
      ViewArray<IntView>
        vstart    = make_view_array(home,    start),
        vend      = make_view_array(home,      end);

      // There is only the value-consistent propagator for this constraint
      GECODE_ES_FAIL(home,(Cumulatives::Val<
                           typename ViewType<Machine>::Result,
                           typename ViewType<Duration>::Result,
                           typename ViewType<Height>::Result,
                           IntView>::post(home, vmachine,vstart, vduration,
                                          vend, vheight,limit, at_most)));

      // By definition, s+d=e should hold.
      // We enforce this using basic linear constraints, since the
      // sweep-algorithm does not check for it.
      for (int i = start.size(); i--; ) {
        sum(home, start[i], duration[i], end[i]);
      }
    }
  }


  void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

  void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel cl, PropVar) {
    post_cumulatives(home, machine, start, duration, end,
                     height, limit, at_most, cl);
  }

}

// STATISTICS: int-post

