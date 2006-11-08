/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include "gecode/minimodel.hh"

namespace Gecode {

  namespace MiniModel {

    /*
     * Operations for linear expressions
     *
     */

    bool
    LinExpr::Node::decrement(void) {
      if (--use == 0) {
        if (left != NULL) {
          if (left->decrement())
            delete left;
          if (right->decrement())
            delete right;
        }
        return true;
      }
      return false;
    }

    int
    LinExpr::Node::fill(Int::Linear::Term<Int::IntView> t[], int i, int m) const {
      if (left != NULL) {
        return right->fill(t, left->fill(t, i, signLeft*m), signRight*m);
      } else {
        t[i].a=m*a; t[i].x=x;
        return i+1;
      }
    }

    void
    LinExpr::post(Space* home, IntRelType irt, IntConLevel icl) const {
      GECODE_AUTOARRAY(Int::Linear::Term<Int::IntView>, ts, n);
      (void) ax->fill(ts,0,sign);
      Int::Linear::post(home, ts, n, irt, sign*-c, icl);
    }

    void
    LinExpr::post(Space* home, IntRelType irt, const BoolVar& b) const {
      GECODE_AUTOARRAY(Int::Linear::Term<Int::IntView>, ts, n);
      (void) ax->fill(ts,0,sign);
      Int::Linear::post(home, ts, n, irt, sign*-c, b);
    }

    IntVar
    LinExpr::post(Space* home, IntConLevel icl) const {
      GECODE_AUTOARRAY(Int::Linear::Term<Int::IntView>, ts, n+1);
      (void) ax->fill(ts,0,sign);
      double min = sign*-c;
      double max = sign*-c;
      for (int i=n; i--; )
        if (ts[i].a > 0) {
          min += ts[i].a*ts[i].x.min();
          max += ts[i].a*ts[i].x.max();
        } else {
          max += ts[i].a*ts[i].x.min();
          min += ts[i].a*ts[i].x.max();
        }
      if (min < Limits::Int::int_min)
        min = Limits::Int::int_min;
      if (max > Limits::Int::int_max)
        max = Limits::Int::int_max;
      IntVar x(home, static_cast<int>(min), static_cast<int>(max));
      ts[n].x = x;
      ts[n].a = -1;
      Int::Linear::post(home, ts, n+1, IRT_EQ, sign*-c, icl);
      return x;
    }


  }

}

// STATISTICS: minimodel-any
